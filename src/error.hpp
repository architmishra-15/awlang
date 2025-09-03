#pragma once
#include <string>
#include <vector>
#include <iostream>

enum class ErrorType {
    LEXICAL_ERROR,
    SYNTAX_ERROR,
    SEMANTIC_ERROR,
    CODEGEN_ERROR,
    WARNING
};

struct CompilerError {
    ErrorType type;
    std::string message;
    std::string suggestion;  // Helpful suggestion for fixing the error
    int line;
    int column;
    int endColumn;  // For highlighting ranges
    std::string filename;
    
    CompilerError(ErrorType t, const std::string& msg, int l, int c, const std::string& file = "", 
                  const std::string& sug = "", int endCol = -1)
        : type(t), message(msg), suggestion(sug), line(l), column(c), 
          endColumn(endCol == -1 ? c : endCol), filename(file) {}
};

// ANSI color codes for terminal output
namespace Colors {
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";
    
    // Background colors
    const std::string BG_RED = "\033[41m";
    const std::string BG_YELLOW = "\033[43m";
}

class ErrorHandler {
private:
    std::vector<CompilerError> errors;
    std::vector<std::string> sourceLines;  // Store source lines for context
    std::string currentFilename;
    bool hasErrors;
    bool hasWarnings;
    
    void printErrorHeader(const CompilerError& error) const;
    void printSourceContext(const CompilerError& error) const;
    void printSuggestion(const CompilerError& error) const;
    std::string getErrorTypeString(ErrorType type) const;
    std::string getErrorTypeColor(ErrorType type) const;
    
public:
    ErrorHandler() : hasErrors(false), hasWarnings(false) {}
    
    void setSourceContent(const std::string& content, const std::string& filename);
    void addError(ErrorType type, const std::string& message, int line, int column, 
                  const std::string& suggestion = "", int endColumn = -1);
    void addLexicalError(const std::string& message, int line, int column, 
                        const std::string& suggestion = "", int endColumn = -1);
    void addSyntaxError(const std::string& message, int line, int column, 
                       const std::string& suggestion = "", int endColumn = -1);
    void addSemanticError(const std::string& message, int line, int column, 
                         const std::string& suggestion = "", int endColumn = -1);
    void addWarning(const std::string& message, int line, int column, 
                   const std::string& suggestion = "", int endColumn = -1);
    
    bool hasAnyErrors() const { return hasErrors; }
    bool hasAnyWarnings() const { return hasWarnings; }
    size_t getErrorCount() const;
    size_t getWarningCount() const;
    
    void printErrors() const;
    void clear();
    
    const std::vector<CompilerError>& getErrors() const { return errors; }
};

// Global error handler instance
extern ErrorHandler g_errorHandler;

// Function to get error handler instance (safer than global)
ErrorHandler& getErrorHandler();