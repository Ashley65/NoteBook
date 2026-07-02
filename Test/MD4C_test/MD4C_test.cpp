#include <iostream>
#include <QString>
#include <QStringList>
#include "MD4C/mdAPI.h"

int main(int argc, char *argv[])
{
    std::cout << "--- Starting MD4C API Test ---" << std::endl;

    QString sampleMarkdown = R"(
# Project Architecture
Welcome to the [[ChronoTasks]] design document!

Here is a list of core components:
* The frontend uses [[QML]] and C++.
* Data is stored via [[SQLite Database]].

### Related Notes
Don't forget to check out the [[Habits MOC]] and the [[010 Mindsets MOC]].

This is just a normal link: [Google](https://google.com). It should NOT be extracted.
    )";

    std::cout << "Parsing markdown for WikiLinks..." << std::endl;
    QStringList extractedLinks = mdAPI::extractMentions(sampleMarkdown);

    std::cout << "Found " << extractedLinks.size() << " links:" << std::endl;

    for (const QString& link : extractedLinks) {
        std::cout << "  -> " << link.toStdString() << std::endl;
    }

    std::cout << "--- Test Complete ---" << std::endl;
    return 0;
}
