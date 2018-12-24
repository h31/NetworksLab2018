import Foundation
import SWXMLHash

fileprivate enum ConverterError: String, Error {
    case badInput
    case elementNotFound
}

final class XMLToYAMLConverter {
    
    private init() {
        fatalError("XMLToYAMLConverter should not be initialized")
    }

    static func convertToYAML(fromXML xml: String) throws -> String {
        let parsedXML = SWXMLHash.parse(xml)
        
        guard let rootIndexer = parsedXML.children.first else {
            throw ConverterError.badInput
        }
        
        return try process(indexer: rootIndexer)
    }
    
}

// MARK: - Private converter implementation

private extension XMLToYAMLConverter {
    
    static func process(indexer: XMLIndexer, level: Int = 0, shouldPrintName: Bool = true) throws -> String {
        guard let element = indexer.element else {
            throw ConverterError.elementNotFound
        }
        
        let childrenDictionary = try indexer.children.reduce(into: [String: [XMLIndexer]]()) { dictionary, indexer in
            guard let element = indexer.element else {
                throw ConverterError.elementNotFound
            }
            
            var indexers = dictionary[element.name] ?? []
            indexers.append(indexer)
            
            dictionary[element.name] = indexers
        }
        
        var output = String(repeating: " ", count: level)
        output += shouldPrintName ? "\(element.name): " : " "
        output += childrenDictionary.isEmpty ? element.text : ""
        
        if output.trimmingCharacters(in: [" "]).isEmpty {
            output = ""
        } else {
            output += "\r\n"
        }
        
        for pair in childrenDictionary {
            if pair.value.count > 1 {
                output += String(repeating: " ", count: level + 1) + "\(pair.key):\r\n"
                for indexer in pair.value {
                    output += String(repeating: " ", count: level + 2) + "-\r\n"
                    output += try process(indexer: indexer, level: level + 2, shouldPrintName: false)
                }
            } else {
                output += try process(indexer: pair.value.first!, level: level + 1)
            }
        }
        
        return output
    }
    
}
