import Foundation

struct Logger {
    
    let name: String
    var enabled: Bool
    
    init(name: String, enabled: Bool = true) {
        self.name = name
        self.enabled = enabled
    }
    
    let dateFormatter: DateFormatter = {
        let dateFormatter = DateFormatter()
        dateFormatter.dateStyle = .short
        dateFormatter.timeStyle = .medium
        return dateFormatter
    }()
    
    func log(_ data: Any) {
        guard enabled else {
            return
        }
        
        print("\(name) \(dateFormatter.string(from: Date())): \(data)")
    }
    
}
