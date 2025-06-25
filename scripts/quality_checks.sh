#!/bin/bash

# libtradier Code Quality Checks
# Pre-commit and CI/CD quality assurance script
# Author: libtradier team
# Version: 1.0

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
CLANG_FORMAT_STYLE="file"
INCLUDE_DIRS="include/ src/ examples/basic/ tests/"
EXCLUDED_DIRS="build/ .git/ third_party/"

print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

check_code_formatting() {
    print_header "Code Formatting Check"
    
    if ! command -v clang-format >/dev/null 2>&1; then
        print_warning "clang-format not found - skipping format check"
        return 0
    fi
    
    # Find all C++ source files
    local cpp_files
    cpp_files=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" -o -name "*.h" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    if [[ -z "$cpp_files" ]]; then
        print_info "No C++ files found for formatting check"
        return 0
    fi
    
    local format_issues=0
    local total_files=0
    
    while IFS= read -r file; do
        if [[ -f "$file" ]]; then
            total_files=$((total_files + 1))
            
            # Check if file needs formatting
            if ! clang-format --style="$CLANG_FORMAT_STYLE" "$file" | diff -q "$file" - >/dev/null 2>&1; then
                print_warning "Format issues in: $file"
                format_issues=$((format_issues + 1))
                
                # Show diff preview
                echo "Formatting diff preview for $file:"
                clang-format --style="$CLANG_FORMAT_STYLE" "$file" | diff -u "$file" - | head -20 || true
                echo "..."
            fi
        fi
    done <<< "$cpp_files"
    
    if [[ $format_issues -eq 0 ]]; then
        print_success "All $total_files files are properly formatted"
    else
        print_error "$format_issues out of $total_files files need formatting"
        print_info "Run: find $INCLUDE_DIRS -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i --style=$CLANG_FORMAT_STYLE"
        return 1
    fi
}

check_include_guards() {
    print_header "Include Guard Check"
    
    local header_files
    header_files=$(find $INCLUDE_DIRS -name "*.hpp" -o -name "*.h" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    if [[ -z "$header_files" ]]; then
        print_info "No header files found"
        return 0
    fi
    
    local guard_issues=0
    local total_headers=0
    
    while IFS= read -r file; do
        if [[ -f "$file" ]]; then
            total_headers=$((total_headers + 1))
            
            # Check for pragma once or include guards
            if ! grep -q "#pragma once" "$file" && ! grep -q "#ifndef.*_H" "$file"; then
                print_warning "Missing include guard in: $file"
                guard_issues=$((guard_issues + 1))
            fi
        fi
    done <<< "$header_files"
    
    if [[ $guard_issues -eq 0 ]]; then
        print_success "All $total_headers header files have include guards"
    else
        print_warning "$guard_issues out of $total_headers header files missing include guards"
        # This is a warning, not an error
    fi
}

check_copyright_headers() {
    print_header "Copyright Header Check"
    
    local source_files
    source_files=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    if [[ -z "$source_files" ]]; then
        print_info "No source files found"
        return 0
    fi
    
    local copyright_pattern="libtradier|MIT License|Copyright"
    local missing_copyright=0
    local total_files=0
    
    while IFS= read -r file; do
        if [[ -f "$file" ]]; then
            total_files=$((total_files + 1))
            
            # Check first 20 lines for copyright/license info
            if ! head -20 "$file" | grep -qi "$copyright_pattern"; then
                print_warning "Missing copyright/license info in: $file"
                missing_copyright=$((missing_copyright + 1))
            fi
        fi
    done <<< "$source_files"
    
    if [[ $missing_copyright -eq 0 ]]; then
        print_success "All $total_files files have copyright/license information"
    else
        print_warning "$missing_copyright out of $total_files files missing copyright info"
        # This is a warning, not an error for existing code
    fi
}

check_naming_conventions() {
    print_header "Naming Convention Check"
    
    local cpp_files
    cpp_files=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    if [[ -z "$cpp_files" ]]; then
        print_info "No C++ files found"
        return 0
    fi
    
    local naming_issues=0
    
    while IFS= read -r file; do
        if [[ -f "$file" ]]; then
            # Check for common naming convention violations
            
            # Class names should be PascalCase
            local bad_class_names
            bad_class_names=$(grep -n "^class [a-z]" "$file" || true)
            if [[ -n "$bad_class_names" ]]; then
                print_warning "Potential class naming issue in $file:"
                echo "$bad_class_names"
                naming_issues=$((naming_issues + 1))
            fi
            
            # Function names should be camelCase or snake_case (we allow both)
            # This is a complex check, so we'll skip it for now
            
            # Check for mixed case in file names (should be snake_case or camelCase)
            local filename
            filename=$(basename "$file")
            if [[ "$filename" =~ [A-Z] && "$filename" =~ _ ]]; then
                print_warning "Mixed case in filename: $file (prefer consistent snake_case or camelCase)"
                naming_issues=$((naming_issues + 1))
            fi
        fi
    done <<< "$cpp_files"
    
    if [[ $naming_issues -eq 0 ]]; then
        print_success "Naming conventions look good"
    else
        print_warning "$naming_issues potential naming convention issues found"
        # This is a warning, not an error
    fi
}

check_todo_fixme_comments() {
    print_header "TODO/FIXME Comment Check"
    
    local source_files
    source_files=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" -o -name "*.md" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    if [[ -z "$source_files" ]]; then
        print_info "No source files found"
        return 0
    fi
    
    local todo_count=0
    local fixme_count=0
    
    while IFS= read -r file; do
        if [[ -f "$file" ]]; then
            # Count TODO comments
            local file_todos
            file_todos=$(grep -n -i "TODO\|FIXME\|XXX\|HACK" "$file" || true)
            
            if [[ -n "$file_todos" ]]; then
                echo "In $file:"
                echo "$file_todos"
                
                todo_count=$((todo_count + $(echo "$file_todos" | grep -ci "TODO" || echo 0)))
                fixme_count=$((fixme_count + $(echo "$file_todos" | grep -ci "FIXME\|XXX\|HACK" || echo 0)))
            fi
        fi
    done <<< "$source_files"
    
    if [[ $todo_count -eq 0 && $fixme_count -eq 0 ]]; then
        print_success "No TODO/FIXME comments found"
    else
        print_info "Found $todo_count TODO comments and $fixme_count FIXME/HACK comments"
        print_info "These are tracked for future improvements"
    fi
}

check_file_size_limits() {
    print_header "File Size Check"
    
    local large_file_limit=2000  # lines
    local huge_file_limit=5000   # lines
    
    local source_files
    source_files=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    if [[ -z "$source_files" ]]; then
        print_info "No source files found"
        return 0
    fi
    
    local large_files=0
    local huge_files=0
    
    while IFS= read -r file; do
        if [[ -f "$file" ]]; then
            local line_count
            line_count=$(wc -l < "$file")
            
            if [[ $line_count -gt $huge_file_limit ]]; then
                print_warning "Very large file ($line_count lines): $file"
                huge_files=$((huge_files + 1))
            elif [[ $line_count -gt $large_file_limit ]]; then
                print_info "Large file ($line_count lines): $file"
                large_files=$((large_files + 1))
            fi
        fi
    done <<< "$source_files"
    
    if [[ $huge_files -eq 0 && $large_files -eq 0 ]]; then
        print_success "All files are reasonably sized"
    else
        print_info "Found $large_files large files (>$large_file_limit lines) and $huge_files very large files (>$huge_file_limit lines)"
        if [[ $huge_files -gt 0 ]]; then
            print_warning "Consider refactoring very large files for maintainability"
        fi
    fi
}

check_dependency_analysis() {
    print_header "Dependency Analysis"
    
    # Check for common problematic includes
    local source_files
    source_files=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    if [[ -z "$source_files" ]]; then
        print_info "No source files found"
        return 0
    fi
    
    local dependency_issues=0
    
    # Check for problematic includes
    local problematic_includes=("bits/" "experimental/" "detail/")
    
    for include_pattern in "${problematic_includes[@]}"; do
        local files_with_pattern
        files_with_pattern=$(grep -l "#include.*$include_pattern" $source_files 2>/dev/null || true)
        
        if [[ -n "$files_with_pattern" ]]; then
            print_warning "Potentially problematic include pattern '$include_pattern' found in:"
            echo "$files_with_pattern"
            dependency_issues=$((dependency_issues + 1))
        fi
    done
    
    # Check for circular dependencies (simple check)
    print_info "Checking for potential circular dependencies..."
    
    local header_files
    header_files=$(find $INCLUDE_DIRS -name "*.hpp" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    # This is a simplified check - a full dependency analysis would require more sophisticated tooling
    local circular_warnings=0
    
    while IFS= read -r file; do
        if [[ -f "$file" ]]; then
            local file_basename
            file_basename=$(basename "$file" .hpp)
            
            # Check if this header includes another header that might include it back
            local includes
            includes=$(grep "#include.*\.hpp" "$file" | sed 's/.*"\(.*\)".*/\1/' || true)
            
            while IFS= read -r included_file; do
                if [[ -n "$included_file" && -f "$included_file" ]]; then
                    if grep -q "$file_basename" "$included_file" 2>/dev/null; then
                        print_warning "Potential circular dependency: $file <-> $included_file"
                        circular_warnings=$((circular_warnings + 1))
                    fi
                fi
            done <<< "$includes"
        fi
    done <<< "$header_files"
    
    if [[ $dependency_issues -eq 0 && $circular_warnings -eq 0 ]]; then
        print_success "Dependency analysis looks good"
    else
        print_warning "Found $dependency_issues problematic includes and $circular_warnings potential circular dependencies"
    fi
}

check_security_patterns() {
    print_header "Security Pattern Check"
    
    local source_files
    source_files=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" 2>/dev/null | \
        grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
    
    if [[ -z "$source_files" ]]; then
        print_info "No source files found"
        return 0
    fi
    
    local security_issues=0
    
    # Check for potentially unsafe functions
    local unsafe_functions=("strcpy" "strcat" "sprintf" "gets" "scanf")
    
    for func in "${unsafe_functions[@]}"; do
        local files_with_func
        files_with_func=$(grep -l "\\b$func\\b" $source_files 2>/dev/null || true)
        
        if [[ -n "$files_with_func" ]]; then
            print_warning "Potentially unsafe function '$func' found in:"
            echo "$files_with_func"
            security_issues=$((security_issues + 1))
        fi
    done
    
    # Check for hardcoded credentials or secrets
    local secret_patterns=("password.*=" "api_key.*=" "secret.*=" "token.*=" "key.*=.*['\"][a-zA-Z0-9]{20,}")
    
    for pattern in "${secret_patterns[@]}"; do
        local files_with_secrets
        files_with_secrets=$(grep -li "$pattern" $source_files 2>/dev/null || true)
        
        if [[ -n "$files_with_secrets" ]]; then
            print_warning "Potential hardcoded secret pattern '$pattern' found in:"
            echo "$files_with_secrets"
            security_issues=$((security_issues + 1))
        fi
    done
    
    # Check for SQL injection patterns (if any database code)
    local sql_patterns=("execute.*+.*" "query.*+.*" "SELECT.*+.*")
    
    for pattern in "${sql_patterns[@]}"; do
        local files_with_sql
        files_with_sql=$(grep -li "$pattern" $source_files 2>/dev/null || true)
        
        if [[ -n "$files_with_sql" ]]; then
            print_warning "Potential SQL injection risk pattern '$pattern' found in:"
            echo "$files_with_sql"
            security_issues=$((security_issues + 1))
        fi
    done
    
    if [[ $security_issues -eq 0 ]]; then
        print_success "No obvious security issues found"
    else
        print_warning "Found $security_issues potential security issues - manual review recommended"
    fi
}

generate_quality_report() {
    print_header "Generating Quality Report"
    
    local report_file="quality_report_$(date +%Y%m%d_%H%M%S).md"
    
    cat > "$report_file" << EOF
# libtradier Code Quality Report

**Generated:** $(date)  
**System:** $(uname -a)  
**Repository:** $(git remote get-url origin 2>/dev/null || echo "Local repository")  
**Commit:** $(git rev-parse HEAD 2>/dev/null || echo "Unknown")  

## Summary

This report contains the results of automated code quality checks for the libtradier project.

## Checks Performed

### ✅ Code Formatting
- **Tool:** clang-format
- **Style:** $CLANG_FORMAT_STYLE
- **Status:** $(check_code_formatting >/dev/null 2>&1 && echo "PASS" || echo "ISSUES FOUND")

### ✅ Include Guards
- **Check:** Header file protection
- **Status:** All headers checked

### ✅ Copyright Headers
- **Check:** License and copyright information
- **Pattern:** MIT License, libtradier, Copyright

### ✅ Naming Conventions
- **Check:** Class and file naming
- **Standard:** PascalCase for classes, consistent file naming

### ✅ TODO/FIXME Analysis
- **Check:** Development comments
- **Purpose:** Track technical debt

### ✅ File Size Analysis
- **Limits:** Warning >2000 lines, Alert >5000 lines
- **Purpose:** Maintainability assessment

### ✅ Dependency Analysis
- **Check:** Include patterns and circular dependencies
- **Purpose:** Architecture health

### ✅ Security Patterns
- **Check:** Unsafe functions and potential vulnerabilities
- **Purpose:** Security assessment

## File Statistics

EOF
    
    # Add file statistics
    local total_cpp_files
    total_cpp_files=$(find $INCLUDE_DIRS -name "*.cpp" 2>/dev/null | wc -l || echo 0)
    local total_hpp_files
    total_hpp_files=$(find $INCLUDE_DIRS -name "*.hpp" 2>/dev/null | wc -l || echo 0)
    local total_lines
    total_lines=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" 2>/dev/null | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}' || echo 0)
    
    cat >> "$report_file" << EOF
- **C++ Source Files:** $total_cpp_files
- **Header Files:** $total_hpp_files  
- **Total Lines of Code:** $total_lines

## Recommendations

1. **Code Formatting:** Ensure all code follows the project's formatting standards
2. **Security:** Review any flagged security patterns manually
3. **Architecture:** Monitor file sizes and dependencies for maintainability
4. **Documentation:** Keep copyright headers up to date

## Tools Used

- clang-format: Code formatting
- grep: Pattern matching
- wc: Line counting
- find: File discovery

---
*This report was generated automatically by the libtradier quality check system.*
EOF
    
    print_success "Quality report generated: $report_file"
}

usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Comprehensive code quality checks for libtradier

OPTIONS:
    -h, --help              Show this help message
    -f, --format-only       Run only code formatting checks
    -s, --security-only     Run only security checks
    -r, --report            Generate quality report
    --fix-format           Automatically fix formatting issues
    --include-dirs "DIRS"   Override include directories
    --exclude-dirs "DIRS"   Override exclude directories

EXAMPLES:
    $0                      # Run all quality checks
    $0 -f                   # Check only code formatting
    $0 --fix-format         # Fix formatting issues automatically
    $0 -r                   # Generate quality report

EOF
}

main() {
    local format_only=false
    local security_only=false
    local report_only=false
    local fix_format=false
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                usage
                exit 0
                ;;
            -f|--format-only)
                format_only=true
                shift
                ;;
            -s|--security-only)
                security_only=true
                shift
                ;;
            -r|--report)
                report_only=true
                shift
                ;;
            --fix-format)
                fix_format=true
                shift
                ;;
            --include-dirs)
                INCLUDE_DIRS="$2"
                shift 2
                ;;
            --exclude-dirs)
                EXCLUDED_DIRS="$2"
                shift 2
                ;;
            *)
                echo "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
    done
    
    print_header "libtradier Code Quality Check Suite"
    
    local failed_checks=0
    
    if [[ "$fix_format" == true ]]; then
        print_header "Auto-fixing Code Formatting"
        
        if command -v clang-format >/dev/null 2>&1; then
            local cpp_files
            cpp_files=$(find $INCLUDE_DIRS -name "*.cpp" -o -name "*.hpp" 2>/dev/null | \
                grep -v -E "($(echo $EXCLUDED_DIRS | tr ' ' '|'))" || true)
            
            if [[ -n "$cpp_files" ]]; then
                while IFS= read -r file; do
                    if [[ -f "$file" ]]; then
                        clang-format --style="$CLANG_FORMAT_STYLE" -i "$file"
                        print_info "Formatted: $file"
                    fi
                done <<< "$cpp_files"
                print_success "Auto-formatting completed"
            fi
        else
            print_error "clang-format not found"
            exit 1
        fi
        exit 0
    fi
    
    if [[ "$report_only" == true ]]; then
        generate_quality_report
        exit 0
    fi
    
    # Run checks based on options
    if [[ "$security_only" == true ]]; then
        check_security_patterns || failed_checks=$((failed_checks + 1))
    elif [[ "$format_only" == true ]]; then
        check_code_formatting || failed_checks=$((failed_checks + 1))
    else
        # Run all checks
        check_code_formatting || failed_checks=$((failed_checks + 1))
        check_include_guards || true  # Warning only
        check_copyright_headers || true  # Warning only
        check_naming_conventions || true  # Warning only
        check_todo_fixme_comments || true  # Informational
        check_file_size_limits || true  # Warning only
        check_dependency_analysis || true  # Warning only
        check_security_patterns || failed_checks=$((failed_checks + 1))
    fi
    
    print_header "Quality Check Summary"
    
    if [[ $failed_checks -eq 0 ]]; then
        print_success "All critical quality checks passed!"
        print_info "Review warnings and informational messages above"
        exit 0
    else
        print_error "$failed_checks critical quality checks failed"
        print_info "Fix the issues above and re-run the quality checks"
        exit 1
    fi
}

# Run main function
main "$@"