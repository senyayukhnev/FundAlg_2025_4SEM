
#include "../include/logger.h"


int main() {
    Logger * logger = Logger::Builder()
            .addHandler(std::make_unique<FileLoggerHandler>("test.log"))
            .addHandler(std::make_unique<StreamLoggerHandler>(std::cout))
            .setName("Logger")
            .build();
    for (int i = 0; i < 100; ++i) {
        logger->Log(static_cast<Logger::LevelLogger>(rand() % 5), "blabla");
    }
}
