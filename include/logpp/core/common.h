
#ifndef LOGPP_API_H
#define LOGPP_API_H

#ifdef LOGPP_STATIC_DEFINE
#  define LOGPP_API
#  define LOGPP_NO_EXPORT
#else
#  ifndef LOGPP_API
#    ifdef logpp_EXPORTS
        /* We are building this library */
#      define LOGPP_API 
#    else
        /* We are using this library */
#      define LOGPP_API 
#    endif
#  endif

#  ifndef LOGPP_NO_EXPORT
#    define LOGPP_NO_EXPORT 
#  endif
#endif

#ifndef LOGPP_DEPRECATED
#  define LOGPP_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef LOGPP_DEPRECATED_EXPORT
#  define LOGPP_DEPRECATED_EXPORT LOGPP_API LOGPP_DEPRECATED
#endif

#ifndef LOGPP_DEPRECATED_NO_EXPORT
#  define LOGPP_DEPRECATED_NO_EXPORT LOGPP_NO_EXPORT LOGPP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LOGPP_NO_DEPRECATED
#    define LOGPP_NO_DEPRECATED
#  endif
#endif

#endif /* LOGPP_API_H */
