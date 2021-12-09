// Simple calculator.
//
#include<iostream>

#include<vector>
#include<map>
#include<set>

#define DEBUG_LOGGING

// Parse unit types. Empty for fail value.
enum tokens
{
    VALUE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    OPERATION,
    EMPTY
};

// Main parse unit.
typedef struct Token
{
    tokens type;
    int64_t value;
}
Token;

// Help function for pretty output of parsing result.
std::ostream& operator<<(std::ostream& os, Token& token)
{
    if (token.type == VALUE)
        os << "[ VALUE, " << token.value << " ]";
    else if (token.type == OPERATION)
        os << "[ OPERATION, " << (char)token.value << " ]";
    else if (token.type == LEFT_BRACKET)
        os << "[ LEFT_BRACKET ]";
    else if (token.type == RIGHT_BRACKET)
        os << "[ RIGHT_BRACKET ]";
    else
        os << "[ UNRECONIZED_TOKEN ]";

    return os;
}

// Namespace for different sequences.
namespace sequence
{
// Proper parsing pair, commutative.
std::set<std::pair<int, int>> commutative = {
    {OPERATION, VALUE}
};

// Also proper parsing pair, but the order is matter.
std::set<std::pair<int, int>> not_commutative = {
    {OPERATION, LEFT_BRACKET},
    {RIGHT_BRACKET, OPERATION},
    {LEFT_BRACKET, VALUE},
    {VALUE, RIGHT_BRACKET},
    {RIGHT_BRACKET, RIGHT_BRACKET},
    {LEFT_BRACKET, LEFT_BRACKET}
};

// Simple classificator.
std::map<char, tokens> classification = {
    {'+', OPERATION},
    {'-', OPERATION},
    {'*', OPERATION},
    {'/', OPERATION},
    {'(', LEFT_BRACKET},
    {')', RIGHT_BRACKET}
};

// Check, if char is a digit. Looks like some kostil.
std::string s_digits = "1234567890";
std::set<char> digits(s_digits.begin(), s_digits.end());
}

// Utils for strings and chars manipulating. Cast to ints, check validness, etc.
namespace utils
{
inline int64_t cast_int(std::string& value) 
{
       int64_t result = 0;
       for (auto ptr = value.begin(); ptr != value.end(); ptr++)
           result = result * 10 + (*ptr - '0');
       return result;
}

// Check tokens pair for correctness.
inline bool check_pair(tokens& first, tokens& second)
{
    return (sequence::commutative.count({first, second}) || \
            sequence::commutative.count({second, first}) || \
            sequence::not_commutative.count({first, second}));
}

// Will check brackets sequence in input.
inline bool brackets_check(std::vector<Token>::iterator from,
                           std::vector<Token>::iterator to)
{
    int64_t balance = 0;
    while (from != to) {
        if (from->type == LEFT_BRACKET)
            balance++;
        else if (from->type == RIGHT_BRACKET)
            balance--;
        if (balance < 0)
            return false;
        from++;
    }
    return (balance == 0);
}

// Check input for it's correctness.
inline bool validate_input(std::vector<Token>::iterator from,
                           std::vector<Token>::iterator to) 
{
    if (!brackets_check(from, to))
        return false;

    std::vector<Token>::iterator pred = from,
                                 cur  = from;
    do {
        if (pred != cur && !check_pair(pred->type, cur->type))
            return false;
        pred = cur;
        cur++;
    } while (cur != to);
    return true;
}
}

// Macro for simplify the operations declaration process.
#define def_operation(key, oper)\
    { key, [](int64_t& first, int64_t& second)\
        { return first oper second; }}

// Binding chars to functions. This will make parsing easier, i swear.
namespace function
{
std::map<char, int64_t(*)(int64_t&, int64_t&)>
    operations = {
        def_operation('+', +),
        def_operation('-', -),
        def_operation('*', *),
        def_operation('/', /)
    };
}

// This macro is just for logging.
#define EXPECT_FAIL(e_type, a_type)\
    do {\
        std::cout << "[type_fail]: failed to apply operation. "\
        << "Expected type: " << e_type << " , actual type: " << a_type\
        << std::endl;\
        exit(-1);\
    } while(0)

// Reduction functions for operations.
// first reduce by product, second reduce by sum (divide, minus).
namespace reduction
{
// Do reduction by product/divide (in integers) operation.
std::vector<Token>& product_reduction(std::vector<Token>::iterator from,
                                      std::vector<Token>::iterator to)
{
    static std::vector<Token> result;
    result.clear();

    std::vector<Token>::iterator first  = from,
                                 oper   = first + 1,
                                 second = oper  + 1;

    if (oper == to || second == to) {
        result.push_back(*from);
        return result;
    }

    if (oper->type != OPERATION)
        EXPECT_FAIL(OPERATION, oper->type);

    if (first->type != VALUE || second->type != VALUE)
        EXPECT_FAIL(VALUE, first->type);

    Token temp = {VALUE, 0}; // empty element for sum operation in integers group.

    if (static_cast<char>(oper->value) == '+' ||\
        static_cast<char>(oper->value) == '-') {
        result.push_back(*first);
        result.push_back(*oper);
        temp.value = second->value;
    }
    else
        temp.value = function::operations[static_cast<char>(oper->value)]
            (first->value, second->value); // application.

    oper   = second + 1;
    second = oper   + 1;

    while (oper != to && second != to) {
        if (oper->type != OPERATION)
            EXPECT_FAIL(OPERATION, oper->type);

        if (second->type != VALUE)
            EXPECT_FAIL(VALUE, second->type);

        if (static_cast<char>(oper->value) == '+' ||\
            static_cast<char>(oper->value) == '-') {
            result.push_back(temp);
            result.push_back(*oper);
            temp.value = second->value;
        }
        else
            temp.value = function::operations[static_cast<char>(oper->value)]
                (temp.value, second->value);
        
        oper   = second + 1;
        second = oper   + 1;
    }
    result.push_back(temp);
    return result;
}

// Do reduction by sum/minus operation.
int64_t sum_reduction(std::vector<Token>::iterator from,
                      std::vector<Token>::iterator to)
{
    if (from->type != VALUE)
        EXPECT_FAIL(VALUE, from->type);

    int64_t result                      = from->value;
    std::vector<Token>::iterator oper   = from + 1,
                                 second = from + 2;

    while (second != to && oper != to) {
        if (oper->type != OPERATION)
            EXPECT_FAIL(OPERATION, oper->type);

        if (second->type != VALUE)
            EXPECT_FAIL(VALUE, second->type);

        result = function::operations[static_cast<char>(oper->value)]
            (result, second->value); // application.

        oper   = second + 1;
        second = oper + 1;
    }
    return result;
}
}

// Parse and classify stdin input. Taking string, returning vector of tokens.
std::vector<Token>& parse_input(std::string& input) 
{
    static std::vector<Token> result;
    // Will remove spaces, because we don't need them;
    std::string input_no_space;
    for (auto it = input.begin(); it != input.end(); it++)
        if (*it != ' ') input_no_space.push_back(*it);

    auto make_value = [](std::string& temp) {
        static Token temp_token;
        temp_token = { VALUE, utils::cast_int(temp) };
        // modify argument passed by refference, because this function is very ad-hoc.
        // won't use it anywhere else.
        temp.clear();
        return temp_token;
    };
    
    std::string temp;
    for (auto it = input_no_space.begin(); it != input_no_space.end(); it++) {
       if (sequence::digits.count(*it)) {
           temp.push_back(*it);
           continue;
       }
       else if (temp.size())
           result.push_back(make_value(temp));

       if (sequence::classification.count(*it)) {
           Token temp_token = {sequence::classification[*it], *it};
           result.push_back(temp_token);
       }
       else 
           std::cout << "Unrecognized token: '" << *it
           << "'. Failed to parse properly, ignoring."
           << std::endl;
    }

    if (temp.size())
        result.push_back(make_value(temp));

    return result;
}

int64_t main_reduction(std::vector<Token>::iterator from,
                       std::vector<Token>::iterator to)
{
#ifdef DEBUG_LOGGING
    std::cout << "[call] main_reduction." << std::endl;
#endif
    // Balance is for right brackets sequences check;
    // by default it is one because it's used only when there is
    // atleast one LEFT_BRACKET. 
    int64_t balance                    = 1; 
    bool    have_brackets              = false;
    std::vector<Token>::iterator start = from;

    for (auto it = from; it != to; ++it)
        if (it->type == LEFT_BRACKET) {
            have_brackets = true;
            start = it + 1;
            break;
        }

    if (!have_brackets) {
        std::vector<Token>::iterator f1 = from, t1 = to;
        std::vector<Token> partial = reduction::product_reduction(f1, t1);
#ifdef DEBUG_LOGGING
        std::cout << "[from-to]:values: " << std::endl;
        for (auto it = from; it != to; ++it)
            std::cout << *it << std::endl;
        std::cout << "[main_reduction]:call:debug: have_brackets = false:" << std::endl
        << "[main_reduction]:call:debug: vector<Token>:partial size = "
        << partial.size() << std::endl
        << "[main_reduction]:call:debug: partial:\n";

        for (auto& x : partial)
            std::cout << x << std::endl;
#endif
        return reduction::sum_reduction(partial.begin(),
                                        partial.end());
    }
    // Temp_start for part BEFORE first left bracket.
    // Temp_mid for part inside brackets.
    // Temp_end for part after brackets.
    std::vector<Token> temp_start, temp_mid, temp_end;

    for (auto it = from; it != start-1; ++it)
        temp_start.push_back(*it);

    while (balance != 0 && start != to) {
        if (start->type == RIGHT_BRACKET)
            balance--;
        else if (start->type == LEFT_BRACKET)
            balance++;

        if (balance != 0) {
            temp_mid.push_back(*start);
#ifdef DEBUG_LOGGING
            std::cout << "[main_reduction]:call:debug: inside brackets token: "
            << *start << std::endl;
#endif  
        }
        start++;
    }

    while (start != to) {
        std::cout << "temp_end: " << *start << std::endl;
        temp_end.push_back(*start);
        start++;
    }
    // Recursive reduction of expressions in brackets.
    // Basic rules, right? 
    int64_t mid_result = main_reduction(temp_mid.begin(),
                                        temp_mid.end());

    temp_start.push_back( { VALUE, mid_result } );
    temp_start.insert(temp_start.end(), temp_end.begin(), temp_end.end());
#ifdef DEBUG_LOGGING
    std::cout << "[main_reduction]:call:debug: temp_start:\n";
    for (auto& x : temp_start)
        std::cout << x << std::endl;
#endif
    // Concate them and reduce again.
    return main_reduction(temp_start.begin(),
                          temp_start.end());
}

int main(int argc, char** argv) 
{
    std::string input;
    char input_char;
    while ((input_char = getchar()) != '\n' && input_char != EOF)
        input.push_back(input_char);

    std::vector<Token> parsing_result = parse_input(input);
    bool input_status = utils::validate_input(parsing_result.begin(),
                                              parsing_result.end());

    if (!input_status) {
        std::cout << "[main]: validation failed, terminating."
        << std::endl;
        exit(-1);
    }
    
    int64_t reduction_result = main_reduction(parsing_result.begin(),
                                              parsing_result.end());
    std::cout << reduction_result << std::endl;
    return 0;
}

