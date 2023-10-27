#ifndef SYSCMDLINE_H
#define SYSCMDLINE_H

#ifdef __cplusplus
extern "C" {
#endif

#define SCL_EXPROT

enum SCL_ValueType {
    SCL_VT_Int,
    SCL_VT_Double,
    SCL_VT_String,
};

enum SCL_HelpPosition {
    SCL_HP_Usage,
    SCL_HP_ErrorText,
    SCL_HP_FirstColumn,
    SCL_HP_SecondColumn,
};

enum SCL_PriorLevel {
    SCL_PL_NoPrior,
    SCL_PL_IgnoreMissingSymbols,
    SCL_PL_AutoSetWhenNoSymbols,
    SCL_PL_ExclusiveToArguments,
    SCL_PL_ExclusiveToOptions,
    SCL_PL_ExclusiveToAll,
};

enum SCL_ShortMatchRule {
    SCL_SMR_NoShortMatch,
    SCL_SMR_ShortMatchAll,
    SCL_SMR_ShortMatchSingleChar,
    SCL_SMR_ShortMatchSingleLetter,
};

enum SCL_MessageType {
    SCL_MT_Debug,
    SCL_MT_Information,
    SCL_MT_Healthy,
    SCL_MT_Warning,
    SCL_MT_Critical,
};

enum SCL_ParseError {
    SCL_PE_NoError,
    SCL_PE_UnknownOption,
    SCL_PE_UnknownCommand,
    SCL_PE_MissingOptionArgument,
    SCL_PE_MissingCommandArgument,
    SCL_PE_TooManyArguments,
    SCL_PE_InvalidArgumentValue,
    SCL_PE_InvalidOptionPosition,
    SCL_PE_MissingRequiredOption,
    SCL_PE_OptionOccurTooMuch,
    SCL_PE_ArgumentTypeMismatch,
    SCL_PE_ArgumentValidateFailed,
    SCL_PE_MutuallyExclusiveOptions,
    SCL_PE_PriorOptionWithArguments,
    SCL_PE_PriorOptionWithOptions,
};

enum SCL_ParseOption {
    SCL_PO_Standard = 0,
    SCL_PO_IgnoreCommandCase = 0x1,
    SCL_PO_IgnoreOptionCase = 0x2,
    SCL_PO_AllowUnixGroupFlags = 0x4,
    SCL_PO_AllowDosKeyValueOptions = 0x8,
    SCL_PO_DontAllowUnixKeyValueOptions = 0x10,
};

enum SCL_DisplayOption {
    SCL_DO_Normal = 0,
    SCL_DO_DontShowHelpOnError = 0x1,
    SCL_DO_SkipCorrection = 0x2,
    SCL_DO_DontShowRequiredOptionsOnUsage = 0x4,
    SCL_DO_ShowOptionalOptionsOnUsage = 0x8,
    SCL_DO_ShowOptionsBehindArguments = 0x10,
    SCL_DO_ShowArgumentDefaultValue = 0x20,
    SCL_DO_ShowArgumentExpectedValues = 0x40,
    SCL_DO_ShowArgumentIsRequired = 0x80,
    SCL_DO_ShowOptionIsRequired = 0x100,
    SCL_DO_AlignAllCatalogues = 0x200,
};

enum SCL_OptionType {
    SCL_OT_User,
    SCL_OT_Version,
    SCL_OT_Help,
};

struct SCL_String {
    char *s;
    int size;
};

struct SCL_StringList {
    struct SCL_String *values;
    int size;
};

struct SCL_Value {
    union {
        int i;
        double d;
        struct SCL_String s;
    } data;
    enum SCL_ValueType type;
};

struct SCL_ArgumentValidator {
    int /* result */ (*func)(const struct SCL_String * /* token */,  //
                             struct SCL_Value * /* out */,           //
                             struct SCL_String * /* errorMessage */, //
                             void * /* user */                       //
    );
    void *user;
};

struct SCL_HelpProvider {
    void (*func)(const void * /* this */,         //
                 enum SCL_HelpPosition /* pos */, //
                 int /* displayOptions */,        //
                 void * /* extra */,              //
                 void * /* user */                //
    );
    void *user;
};

struct SCL_ParseResult;

struct SCL_CommandHandler {
    int (*func)(const struct SCL_ParseResult * /* result */, //
                void * /* user */                            //
    );
    void *user;
};

// For COW
struct SCL_SharedBase {
    int ref;
};

struct SCL_Symbol {
    struct SCL_SharedBase shared;
    struct SCL_String description;
    struct SCL_HelpProvider helpProvider;
};

struct SCL_Argument {
    struct SCL_Symbol base;
    struct SCL_String name;
    struct SCL_Value defaultValue;
    int required;
    int multiple;
    struct SCL_String displayName;
    struct SCL_ArgumentValidator validator;

    // Expected Values
    struct SCL_Value *expectedValues;
    int expectedValuesSize;
};

struct SCL_Option {
    struct SCL_Symbol base;
    struct SCL_StringList tokens;
    int required;
    enum SCL_ShortMatchRule shortMatchRule;
    enum SCL_PriorLevel priorLevel;
    int global;
    int maxOccurrence;
    enum SCL_OptionType type; // fallback helper

    // Arguments
    struct SCL_Argument *arguments;
    int argumentsSize;
};

struct SCL_CommandCatalogueItem {
    struct SCL_String title;
    int *indexes;
    int indexesSize;
};

struct SCL_CommandCatalogue {
    struct SCL_SharedBase shared;
    struct SCL_CommandCatalogueItem arg;
    struct SCL_CommandCatalogueItem opt;
    struct SCL_CommandCatalogueItem cmd;
};

struct SCL_MultualExclusiveGroups {
    struct SCL_StringList *groups;
    int size;
};

struct SCL_Command {
    struct SCL_Symbol base;
    struct SCL_String name;
    struct SCL_String version;
    struct SCL_String detailedDescription;

    struct SCL_Option helpOption;
    struct SCL_Option versionOption;

    struct SCL_CommandCatalogue catalogue;
    struct SCL_MultualExclusiveGroups exclusiveGroups;
    struct SCL_CommandHandler handler;

    // Arguments
    struct SCL_Argument *arguments;
    int argumentsSize;

    // Options
    struct SCL_Option *options;
    int optionsSize;

    // Commands
    struct SCL_Command *commands;
    int commandsSize;
};

enum SCL_HelpItemType {
    SCL_HIT_Prologue,
    SCL_HIT_Information,
    SCL_HIT_Warning,
    SCL_HIT_Error,
    SCL_HIT_Description,
    SCL_HIT_Usage,
    SCL_HIT_Arguments,
    SCL_HIT_Options,
    SCL_HIT_Commands,
    SCL_HIT_Epilogue,
    SCL_HIT_User_List,
    SCL_HIT_User_String,
    SCL_HIT_User_Function,
};

enum SCL_SizeType {
    SCL_ST_Indent,
    SCL_ST_Spacing,
    SCL_ST_ConsoleWidth,
};

struct SCL_SizeConfig {
    int sizeConfig[3];
};

enum SCL_TextCategory {
    SCL_TC_ParseError,
    SCL_TC_Title,
    SCL_TC_DefaultCommand,
    SCL_TC_Information,
};

enum SCL_TextCategory_TitleText {
    SCL_TC_TT_Error,
    SCL_TC_TT_Usage,
    SCL_TC_TT_Description,
    SCL_TC_TT_Arguments,
    SCL_TC_TT_Options,
    SCL_TC_TT_Commands,
    SCL_TC_TT_Required,
    SCL_TC_TT_Default,
    SCL_TC_TT_ExpectedValues,
};

enum SCL_TextCategory_CommandText {
    SCL_TC_CT_Version,
    SCL_TC_CT_Help,
};

enum SCL_TextCategory_InformationText {
    SCL_TC_IT_MatchCommand,
};

struct SCL_HelpList {
    struct SCL_String title;
    struct SCL_StringList firstColumn;
    struct SCL_StringList secondColumn;
};

struct SCL_HelpFunction {
    void (*func)(void * /* user */);
    void *user;
};

struct SCL_HelpItem {
    enum SCL_HelpItemType type;
    void (*func)(void * /* data */);
};

struct SCL_HelpLayout {
    struct SCL_SharedBase shared;
    struct SCL_HelpItem *items;
    int itemsSize;
};

struct SCL_Parser {
    struct SCL_SharedBase shared;
    struct SCL_String prologue;
    struct SCL_String epilogue;
    int displayOptions;
    struct SCL_HelpLayout *helpLayout;

    int sizeConfig[3];
    void (*textProvider)(int /* category */, int /* index */, struct SCL_String * /* out */);
};

struct SCL_ParseResult {
    struct SCL_SharedBase shared;
    struct SCL_StringList arguments;
    struct SCL_Parser *parser;
};

SCL_EXPROT void *SCL_Alloc(int size);
SCL_EXPROT void SCL_Free(void *buf);

SCL_EXPROT void SCL_ApplicationPath(struct SCL_String *out);
SCL_EXPROT void SCL_ApplicationDirectory(struct SCL_String *out);
SCL_EXPROT void SCL_ApplicationFileName(struct SCL_String *out);
SCL_EXPROT void SCL_ApplicationName(struct SCL_String *out);
SCL_EXPROT void SCL_CommandLineArguments(struct SCL_StringList *out);

SCL_EXPROT int SCL_Debug(enum SCL_MessageType messageType, int highlight, const char *fmt, ...);
SCL_EXPROT int SCL_Print(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // SYSCMDLINE_H
