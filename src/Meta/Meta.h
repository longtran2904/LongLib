/* date = November 14th 2021 9:59 pm */

#ifndef _META_H
#define _META_H

typedef enum
{
    MetaType_int,
    MetaType_float,
    MetaType_bool,
    MetaType_char,
    
    MetaTypeCount
} MetaType;

typedef enum
{
    MetaFlag_IsPointer = 0x01,
} MemberDefinitionFlag;

typedef struct
{
    MetaType type;
    char* fieldName;
    int flag;
    int offset;
} MemberDefinition;

#define introspect(param)
#ifndef META_GENERATED
#define _CALLER_FILE_ ""
#define _CALLER_LINE_ 0
#else
#define _CALLER_FILE_ __caller_file__
#define _CALLER_LINE_ __caller_line__
#endif

#endif //_META_H
