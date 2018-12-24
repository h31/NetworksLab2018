import XCTest

#if !os(macOS)
public func allTests() -> [XCTestCaseEntry] {
    return [
        testCase(second_taskTests.allTests),
    ]
}
#endif