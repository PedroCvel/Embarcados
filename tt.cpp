#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <curl/curl.h>

const std::string apiKey = "56aa2487148b102db75aabc6cc3ffe82";
const std::string uploadUrl = "https://api.imgbb.com/1/upload";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* buffer) {
    size_t total_size = size * nmemb;
    buffer->append(reinterpret_cast<char*>(contents), total_size);
    return total_size;
}

int main() {
    cv::CascadeClassifier faceCascade;

    // Carregando o classificador pré-treinado para detecção facial
    if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
        std::cerr << "Erro ao carregar o classificador!" << std::endl;
        return -1;
    }

    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Erro ao abrir a câmera!" << std::endl;
        return -1;
    }

    int imageCount = 0;
    const int maxImages = 150;  // Número máximo de imagens a serem armazenadas

    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    while (true) {
        cv::Mat frame;
        cap >> frame;

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.3, 5);

        for (const auto& face : faces) {
            // Salva a imagem contendo a face
            cv::Mat faceImage = frame(face);
            std::string imageName = "face_" + std::to_string(imageCount % maxImages) + ".jpg";
            cv::imwrite(imageName, faceImage);
            std::cout << "Imagem salva como: " << imageName << std::endl;

            // Envia a imagem para o imgbb.com
            std::ifstream imageFile(imageName, std::ios::binary);
            if (imageFile.is_open()) {
                std::string content((std::istreambuf_iterator<char>(imageFile)), std::istreambuf_iterator<char>());

                if (curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, uploadUrl.c_str());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ("key=" + apiKey + "&image=" + content).c_str());
                    
                    res = curl_easy_perform(curl);

                    if (res != CURLE_OK)
                        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                }

                imageFile.close();
            }

            // ...

            imageCount++;
        }

        // ...

        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    cap.release();

    return 0;
}
