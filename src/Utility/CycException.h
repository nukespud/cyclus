// CycException.h
#if !defined(_GEN_EXCEPTION)
#define _GEN_EXCEPTION

#include <exception>
#include <string>

/**
 * @class CycException
 * @brief A generic mechanism to manually manage exceptions
 */
class CycException: public std::exception {

protected:
    /**
     * @brief  The message associated with this exception.
     */
    std::string myMessage_;
    
    /**
     * @brief  A string to prepend to all message of this class.
     */
    static std::string prepend_;
    
public:
    /**
     * @brief  Constructs a new CycException with the default message.
     */
    CycException();
    
    /**
     * @brief Constructs a new CycException with a provided message
     *
     * @param msg the message
     */
    CycException(std::string  msg);
    
    /**
     * @brief Returns the error message associated with this CycException.
     *
     * @return the message
     */
    virtual const char* what() const throw();
    
    /**
     * @brief Destroys this CycException.
     */
    virtual ~CycException() throw();
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CycIndexException: public CycException {
  public: CycIndexException(std::string msg) : CycException(msg) {};
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CycIOException: public CycException {
  public: CycIOException(std::string msg) : CycException(msg) {};
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CycMessageException: public CycException {
  public: CycMessageException(std::string msg) : CycException(msg) {};
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CycTypeException: public CycException {
  public: CycTypeException(std::string msg) : CycException(msg) {};
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CycRangeException: public CycException {
  public: CycRangeException(std::string msg) : CycException(msg) {};
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CycParseException: public CycException {
  public: CycParseException(std::string msg) : CycException(msg) {};
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CycOverrideException: public CycException {
  public: CycOverrideException(std::string msg) : CycException(msg) {};
};

#endif


