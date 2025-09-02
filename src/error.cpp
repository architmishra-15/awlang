#include "error.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

// Global error handler instance
ErrorHandler g_errorHandler;

// Function to get error handler instance (safer than global)
ErrorHandler& getErrorHandler() {
    return g_errorHandler;
}

void ErrorHandler::setSourceContent(const std::string& content, const std::string& filename) {
    currentFilename = filename;
    sourceLines.clear();
    
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        sourceLines.push_back(line);
    }
}

void ErrorHandler::addError(ErrorType type, const std::string& message, int line, int column, 
                           const std::string& suggestion, int endColumn) {
    errors.emplace_back(type, message, line, column, currentFilename, suggestion, endColumn);
    hasErrors = true;
}

void ErrorHandler::addLexicalError(const std::string& message, int line, int column, 
                                  const std::string& suggestion, int endColumn) {
    addError(ErrorType::LEXICAL_ERROR, message, line, column, suggestion, endColumn);
}

void ErrorHandler::addSyntaxError(const std::string& message, int line, int column, 
                                 const std::string& suggestion, int endColumn) {
    addError(ErrorType::SYNTAX_ERROR, message, line, column, suggestion, endColumn);
}

void ErrorHandler::addSemanticError(const std::string& message, int line, int column, 
                                   const std::string& suggestion, int endColumn) {
    addError(ErrorType::SEMANTIC_ERROR, message, line, column, suggestion, endColumn);
}

std::string ErrorHandler::getErrorTypeString(ErrorType type) const {
    switch (type) {
        case ErrorType::LEXICAL_ERROR: return "lexical error";
        case ErrorType::SYNTAX_ERROR: return "syntax error";
        case ErrorType::SEMANTIC_ERROR: return "semantic error";
        case ErrorType::CODEGEN_ERROR: return "codegen error";
        default: return "unknown error";
    }
}

std::string ErrorHandler::getErrorTypeColor(ErrorType type) const {
    switch (type) {
        case ErrorType::LEXICAL_ERROR: return Colors::RED;
        case ErrorType::SYNTAX_ERROR: return Colors::RED;
        case ErrorType::SEMANTIC_ERROR: return Colors::YELLOW;
        case ErrorType::CODEGEN_ERROR: return Colors::MAGENTA;
        default: return Colors::RED;
    }
}

void ErrorHandler::printErrorHeader(const CompilerError& error) const {
    std::cerr << Colors::BOLD << getErrorTypeColor(error.type) << "error" << Colors::RESET 
              << Colors::BOLD << ": " << Colors::RESET << error.message << std::endl;
    
    std::cerr << Colors::BLUE << "  --> " << Colors::RESET;
    if (!error.filename.empty()) {
        std::cerr << error.filename << ":";
    }
    std::cerr << error.line << ":" << error.column << std::endl;
}

void ErrorHandler::printSourceContext(const CompilerError& error) const {
    if (sourceLines.empty() || error.line < 1 || error.line > (int)sourceLines.size()) {
        return;
    }
    
    int lineNum = error.line;
    int startLine = std::max(1, lineNum - 2);
    int endLine = std::min((int)sourceLines.size(), lineNum + 2);
    
    // Calculate padding for line numbers
    int maxLineNumWidth = std::to_string(endLine).length();
    
    std::cerr << Colors::BLUE << std::string(maxLineNumWidth + 1, ' ') << " |" << Colors::RESET << std::endl;
    
    // Print context lines
    for (int i = startLine; i <= endLine; i++) {
        bool isErrorLine = (i == lineNum);
        std::string linePrefix = Colors::BLUE + std::to_string(i) + 
                                std::string(maxLineNumWidth - std::to_string(i).length(), ' ') + 
                                " | " + Colors::RESET;
        
        if (isErrorLine) {
            std::cerr << linePrefix << sourceLines[i - 1] << std::endl;
            
            // Print error indicator
            std::cerr << Colors::BLUE << std::string(maxLineNumWidth + 1, ' ') << " | " << Colors::RESET;
            
            // Add spaces to align with error column
            for (int j = 1; j < error.column; j++) {
                std::cerr << " ";
            }
            
            // Print error indicator
            std::cerr << Colors::RED << Colors::BOLD;
            if (error.endColumn > error.column) {
                // Multi-character error
                for (int j = error.column; j <= error.endColumn; j++) {
                    std::cerr << "^";
                }
            } else {
                std::cerr << "^";
            }
            std::cerr << Colors::RESET << std::endl;
        } else {
            std::cerr << linePrefix << sourceLines[i - 1] << std::endl;
        }
    }
    
    std::cerr << Colors::BLUE << std::string(maxLineNumWidth + 1, ' ') << " |" << Colors::RESET << std::endl;
}

void ErrorHandler::printSuggestion(const CompilerError& error) const {
    if (!error.suggestion.empty()) {
        std::cerr << Colors::GREEN << Colors::BOLD << "help: " << Colors::RESET 
                  << Colors::GREEN << error.suggestion << Colors::RESET << std::endl;
    }
}

void ErrorHandler::printErrors() const {
    if (errors.empty()) return;
    
    std::cerr << std::endl;
    
    for (size_t i = 0; i < errors.size(); i++) {
        const auto& error = errors[i];
        
        printErrorHeader(error);
        printSourceContext(error);
        printSuggestion(error);
        
        // Add spacing between errors (except for the last one)
        if (i < errors.size() - 1) {
            std::cerr << std::endl;
        }
    }
    
    // Print summary
    std::cerr << std::endl;
    std::cerr << Colors::RED << Colors::BOLD << "error" << Colors::RESET << ": could not compile `";
    if (!currentFilename.empty()) {
        std::cerr << currentFilename;
    } else {
        std::cerr << "input";
    }
    std::cerr << "` due to " << errors.size() << " previous error";
    if (errors.size() > 1) std::cerr << "s";
    std::cerr << std::endl;
}

void ErrorHandler::clear() {
    errors.clear();
    sourceLines.clear();
    currentFilename.clear();
    hasErrors = false;
}