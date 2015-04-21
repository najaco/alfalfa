#ifndef OPTIONAL_HH
#define OPTIONAL_HH

#include <stdexcept>
#include <cassert>
#include <utility>

template <class T>
class Optional
{
private:
  bool initialized_;
  union { T object_; bool missing_; };

public:
  /* constructor for uninitialized optional */
  /* missing_ field of union is necessary to get rid of "may be used uninitialized" warning */
  Optional() : initialized_( false ), missing_() {}

  /* constructor for initialized optional */
  Optional( T && other ) : initialized_( true ), object_( std::move( other ) ) {}

  /* conditional constructor */
  template <typename... Targs>
  Optional( const bool is_present, Targs&&... Fargs )
    : initialized_( is_present )
  {
    if ( initialized_ ) {
      new( &object_ ) T( std::forward<Targs>( Fargs )... );
    }
  }

  /* move constructor */
  Optional( Optional<T> && other )
    : initialized_( other.initialized_ )
  {
    if ( initialized_ ) {
      new( &object_ ) T( std::move( other.object_ ) );
    }
  }

  /* copy constructor */
  Optional( const Optional<T> & other )
    : initialized_( other.initialized_ )
  {
    if ( initialized_ ) {
      new( &object_ ) T( other.object_ );
    }
  }

  /* initialize in place */
  template <typename... Targs>
  void initialize( Targs&&... Fargs )
  {
    assert( not initialized() );
    new( &object_ ) T( std::forward<Targs>( Fargs )... );
    initialized_ = true;
  }

  /* move assignment operators */
  const Optional & operator=( Optional<T> && other )
  {
    /* destroy if necessary */
    if ( initialized_ ) {
      object_.~T();
    }

    initialized_ = other.initialized_;
    if ( initialized_ ) {
      new( &object_ ) T( std::move( other.object_ ) );
    }
    return *this;
  }

  /* copy assignment operator */
  const Optional & operator=( const Optional<T> & other )
  {
    /* destroy if necessary */
    if ( initialized_ ) {
      object_.~T();
    }

    initialized_ = other.initialized_;
    if ( initialized_ ) {
      new( &object_ ) T( other.object_ );
    }
    return *this;
  }

  /* equality */
  bool operator==( const Optional<T> & other ) const
  {
    if ( initialized_ ) {
      return other.initialized_ ? (get() == other.get()) : false;
    } else {
      return !other.initialized_;
    }
  }

  /* getters */
  bool initialized( void ) const { return initialized_; }
  const T & get( void ) const { assert( initialized() ); return object_; }
  const T & get_or( const T & default_value ) const { return initialized() ? object_ : default_value; }

  T & get( void ) { assert( initialized() ); return object_; }

  /* destructor */
  ~Optional() { if ( initialized() ) { object_.~T(); } }

  void clear( void ) { if ( initialized() ) { object_.~T(); } initialized_ = false; }
};

#endif /* OPTIONAL_HH */
