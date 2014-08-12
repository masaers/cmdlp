cmdlp
=====

A template-based command line parser for c++.


todo
====

  * Add a validation function to options, so that the parser can
    determine whether the option is validly initialized (otherwise it
    needs to be set at the command line), and whether the option it
    valid after parsing (otherwise parsing failed).

  * Since the parser will have to give some information about how the
    user can set valid values, the validator will have to be a custom
    functor with a stringifier.

    
