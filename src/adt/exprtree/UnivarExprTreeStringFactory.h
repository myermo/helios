#ifndef _UNIVAREXPRTREESTRINGFACTORY_H_
#define _UNIVAREXPRTREESTRINGFACTORY_H_


#include <adt/exprtree/UnivarExprTreeNode.h>
#include <adt/exprtree/IExprTreeNodeStringFactory.h>
#include <util/HeliosException.h>

#include <vector>

/**
 * @author Alberto M. Esmoris Pena
 * @version 1.0
 * @brief Class implementing a factory that makes univariate expression trees
 *  from expressions given as strings.
 *
 * The expression strings are expected as infix expressions (infix notation).
 *
 * Infix expression example: \f$a + b\f$
 *
 * Prefix expression example: \f$+ a b\f$
 *
 * Postfix expression example: \f$a b +\f$
 *
 * The core implementation of the expression tree is based on two sets of
 *  logical rules that specify how push and pop operations must be handled.
 *
 * First, when one of the the following logical rules is satisfied, the
 *  algorithm must stop reading and do the corresponding operations to push
 *  new trees built from popped operators and trees.
 *
 * <ol>
 *  <li>Reading a closing priority operator (e.g., ')')</li>
 *  <li>Reading an operator whose priority is less than or equal to the
 *  priority of the operator at the top of the stack of operators</li>
 *  <li>End of reading</li>
 * </ol>
 *
 * Second, when push and pop operations are triggered, they must stop depending
 *  on why they have been triggered.
 *
 * <ol>
 *  <li>If ')' has been read, then all the elements in the stack of operators
 *      must be pop until the corresponding '(' has been reached (inclusive).
 *  </li>
 *  <li>If an operator with priority less than or equal to the priority of the
 *   operator in the top of the stack has been read, then all the operators
 *   in the stack with a priority greater than or equal to read operator must
 *   be popped.
 *   <li>If reading has finished, then all the operators in the stack must be
 *   consumed and consequently there must be a single tree in the stack of
 *   nodes/trees (otherwise the generated expression try will be inconsistent).
 *   </li>
 * </ol>
 *
 * @tparam NumericType The numeric type of expression trees to be built by
 *  the factory
 * @see UnivarExprTreeNode
 * @see IExprTreeNodeStringFactory
 */
template <typename NumericType>
class UnivarExprTreeStringFactory :
        public IExprTreeNodeStringFactory<NumericType, NumericType>{
public:
    // ***  CONSTANTS  *** //
    // ******************* //
    /**
     * @brief The base priority for addition and subtraction operators
     */
    static unsigned int const OP_ADD_BASE_PRIORITY;
    /**
     * @brief The base priority for product and division operators
     */
    static unsigned int const OP_MUL_BASE_PRIORITY;
    /**
     * @brief The base priority for power operators
     */
    static unsigned int const OP_POW_BASE_PRIORITY;
    /**
     * @brief The base priority for functions
     */
    static unsigned int const FUN_BASE_PRIORITY;
    /**
     * @brief The base priority for brackets
     */
    static unsigned int const BRACKET_BASE_PRIORITY;
    /**
     * @brief Expected characters in expressions
     */
    static char const EXPRESSION_CHARS[];
    /**
     * @brief Supported functions by name
     */
    static std::string const FUNCTION_NAMES[];

    // ***  SUB-CLASS  *** //
    // ******************* //
    /**
     * @brief UnivarExprTreeStringFactory sub-class used to represent symbols
     *  during recursive building
     */
    struct Symbol{
        /**
         * @brief The symbol type
         */
        typename UnivarExprTreeNode<NumericType>::SymbolType type;
        /**
         * @brief The symbol as a string
         */
        std::string str;
    };


    // ***  ATTRIBUTES  *** //
    // ******************** //
    /**
     * @brief The base priority at current state
     */
    unsigned int basePriority = 0;
    /**
     * @brief True when the last read has been a priority operator or a
     *  separator, false otherwise
     */
    bool lastReadIsOpenPriorityOrSeparator = false;
    /**
     * @brief The nodes at current state (must be used as a stack)
     */
    std::vector<UnivarExprTreeNode<NumericType> *> nodes;
    /**
     * @brief The operators at current state (must be used as a stack)
     */
    std::vector<Symbol> ops;

    // ***  CONSTRUCTION / DESTRUCTION  *** //
    // ************************************ //
    /**
     * @brief Default constructor for univariate expression tree string
     *  factory
     */
    UnivarExprTreeStringFactory() :
        IExprTreeNodeStringFactory<NumericType, NumericType>(),
        basePriority(0)
    {}
    virtual ~UnivarExprTreeStringFactory() = default;

    // ***  EXPRESSION TREE NODE STRING FACTORY INTERFACE  *** //
    // ******************************************************* //
    /**
     * @brief Make an expression tree from given expression as string
     * @param expr The expression given as a string
     * @return Built univariate expression tree
     */
    IExprTreeNode<NumericType, NumericType> * make(
        std::string const &expr
    ) override;

    // ***  MAKE METHODS  *** //
    // ********************** //
    /**
     * @brief Iteratively built the corresponding expression tree node
     *  from given expression
     * @return Iteratively built univariate expression tree
     */
    virtual IExprTreeNode<NumericType, NumericType> * makeIterative(
        std::string const &expr
    );
    /**
     * @brief Flush the current state of the factory.
     *
     * The flush operation must be called always that one of the following
     *  conditions is satisfied:
     *
     * <ol>
     *  <li>A closing priority operator (parentheses) is read</li>
     *  <li>An operator with priority \f$\leq\f$ top operator on
     *  UnivarExprTreeStringFactory::ops stack is read</li>
     *  <li>End of parsing</li>
     * </ol>
     *
     * @see UnivarExprTreeStringFactory::ops
     */
    virtual void flush();

    // ***  POST-PROCESS METHODS  *** //
    // ****************************** //
    /**
     * @brief Transform all POW nodes that are either the given node or a child
     *  of it into IPOW nodes
     * @param node Node to be optimized by using IPOW whenever possible
     */
    void doIPowOptimization(UnivarExprTreeNode<NumericType> *node);

    // ***  HANDLE METHODS  *** //
    // ************************ //
    /**
     * @brief Handle parsed operator
     * @param symbol Symbol representing the parsed operator
     * @see UnivarExprTreeStringFactory::Symbol
     */
    virtual void handleOp(Symbol const &symbol);
    /**
     * @brief Handle parsed function
     * @param symbol Symbol representing the parsed function
     * @see UnivarExprTreeStringFactory::Symbol
     */
    virtual void handleFun(Symbol const &symbol);


    // ***  PRIORITY METHODS  *** //
    // ************************** //
    /**
     * @brief Calculate the corresponding priority considering current base
     *  priority and given operator
     * @param op Given operator symbol for which priority must be calculated
     * @return Calculated priority
     * @see UnivarExprTreeStringFactory::basePriority
     */
    inline unsigned int calcOpPriority(
        Symbol const &symbol
    ){
        if(symbol.str == "+" || symbol.str == "-")
            return basePriority + OP_ADD_BASE_PRIORITY;
        else if(symbol.str == "*" || symbol.str == "/")
            return basePriority + OP_MUL_BASE_PRIORITY;
        else if(symbol.str == "^")
            return basePriority + OP_POW_BASE_PRIORITY;
        else if(symbol.str == "(") return basePriority;
        else if(symbol.str == "atan2") return basePriority + FUN_BASE_PRIORITY;
        else{
            throw HeliosException(
                "UnivarExprTreeStringFactory::calcOpPriority received an "
                "unexpected operator as input"
            );
        }
    }
    /**
     * @brief Calculate the corresponding priority considering current base
     *  priority and a function
     * @return Calculated priority
     * @see UnivarExprTreeStringFactory::basePriority
     */
    inline unsigned int calcFunPriority()
    {return basePriority + FUN_BASE_PRIORITY;}
    /**
     * @brief Calculate the corresponding priority considering current base
     *  priority and the push of an opening bracket
     * @return Calculated priority
     * @see UnivarExprTreeStringFactory::basePriority
     * @see UnivarExprTreeStringFactory::calcCloseBracketPriority
     */
    inline unsigned int calcOpenBracketPriority()
    {return basePriority + BRACKET_BASE_PRIORITY;}
    /**
     * @brief Calculate the corresponding bracket priority considering current
     *  base priority and the pop of an opening bracket (which happens because
     *  a close bracket has been read)
     * @return Calculated priority
     * @see UnivarExprTreeStringFactory::basePriority
     * @see UnivarExprTreeStringFactory::calcOpenBracketPriority
     */
    inline unsigned int calcCloseBracketPriority()
    {return basePriority - BRACKET_BASE_PRIORITY;}

    // ***  UTIL METHODS  *** //
    // ********************** //
    /**
     * @brief Obtain the first symbol (token) parsed starting at the \f$0\f$
     *  character
     * @param expr The expression to be parsed to obtain a symbol
     * @return First symbol contained in given expression
     */
    Symbol nextSymbol(std::string const &expr);
    /**
     * @brief Prepare the expression string so it can be digested by an
     *  iterative make process.
     *
     * This method implies invoking
     *  UnivarExprTreeStringFactory::cleanExpressionString always.
     *
     * @param expr The expression string to be prepared
     * @return Prepared expression, ready for an iterative make process
     * @see UnivarExprTreeStringFactory::cleanExpressionString
     */
    std::string prepareExpressionString(std::string const &expr);
    /**
     * @brief Clean the expression string from unnecessary characters.
     *
     * This method is also invoked when
     *  UnivarExprTreeStringFactory::prepareExpressionString is invoked.
     *
     * @param expr The expression string to be cleaned
     * @return Cleaned expression
     * @see UnivarExprTreeStringFactory::prepareEpresssionString
     */
    std::string cleanExpressionString(std::string const &expr);
    /**
     * @brief Represent the given number as a string
     * @param x The number to be represented as a string
     * @param precision The amount of decimal digits
     * @return String representation of given number
     */
    std::string stringFromNumber(
        NumericType const x,
        std::streamsize const precision=16
    );
    /**
     * @brief Craft a function symbol from given string
     * @param symstr String representation of a function name
     * @return Crafted function symbol
     */
    Symbol craftFunSymbol(std::string const &symstr);
    /**
     * @brief Craft the NON-NEGATIVE numeric symbol represented by an arbitrary
     *  size string for which the first character is guaranteed to be the start
     *  of the represented number
     * @param expr String such that there is a substring starting at the first
     *  character that represents a number (e.g. "0.567 + 2.1" or "5*3")
     * @return Crafted numeric symbol
     */
    Symbol craftNumSymbol(std::string const &expr);
    /**
     * @brief Check whether the given string is representing a valid operator
     *  (true) or not (false).
     * @param opStr The string to be checked.
     * @return True if given string represents a valid operator, false
     *  otherwise
     */
    inline bool isValidOpString(std::string const &opStr){
        return  opStr == "+"    ||      opStr == "-"     ||     opStr == "*" ||
                opStr == "/"    ||      opStr == "^"     ||     opStr == "(" ||
                opStr == ")"    ||      opStr == "atan2"
        ;
    }
    /**
     * @brief Check whether the given string is representing a valid function
     *  (true) or not (false).
     * @param funStr The string to be checked.
     * @return True if given string represents a valid function, false
     *  otherwise
     */
    inline bool isValidFunString(std::string const &funStr){
        return  funStr == "exp"         ||      funStr == "ln"      ||
                funStr == "sqrt"        ||      funStr == "abs"     ||
                funStr == "cos"         ||      funStr == "sin"     ||
                funStr == "tan"         ||      funStr == "acos"    ||
                funStr == "asin"        ||      funStr == "atan"    ||
                funStr == "cosh"        ||      funStr == "sinh"    ||
                funStr == "tanh";
    }
};

#include <adt/exprtree/UnivarExprTreeStringFactory.tpp>

#endif