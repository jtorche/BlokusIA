#include "XmlConfigReader.h"

#include <QFile>
#include <QXmlStreamReader>

#include<QDebug>

namespace blokusUi
{
    core::flat_hash_map<std::string, std::string> XmlConfigReader::loadConfiguration(
        const std::string& _resourcePath,
        const std::string& _elementName)
    {
        core::flat_hash_map<std::string, std::string> config;

        QFile xmlFile{ _resourcePath.c_str() };
        xmlFile.open(QFile::ReadOnly | QFile::Text);
        QXmlStreamReader reader{ xmlFile.readAll() };
        while (!reader.atEnd() && !reader.hasError())
        {
            if (reader.readNext() == QXmlStreamReader::StartElement
                && reader.name() == _elementName.c_str()
                && reader.attributes().hasAttribute("name"))
            {
                config.insert(
                    { 
                        reader.attributes().value("name").toString().toStdString(), // Resource name
                        reader.readElementText().toStdString()                      // Resource value
                    });
            }
        }

        return config;
    }
}
