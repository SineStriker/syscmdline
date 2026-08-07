/****************************************************************************
 *
 * Regression tests for SysCmdLine.
 *
 * Deliberately free of any test framework, and free of `assert`, so that the
 * suite means the same thing in a release build as it does in a debug one.
 * A failing check is reported and counted. The process exit code is the
 * number of failures.
 *
 ****************************************************************************/

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#  include <io.h>
#  define SCL_DUP   _dup
#  define SCL_DUP2  _dup2
#  define SCL_FILENO _fileno
#else
#  include <unistd.h>
#  define SCL_DUP   dup
#  define SCL_DUP2  dup2
#  define SCL_FILENO fileno
#endif

#include <syscmdline/parser.h>
#include <syscmdline/system.h>

namespace SCL = SysCmdLine;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Harness
// ---------------------------------------------------------------------------

static int g_checks = 0;
static int g_failures = 0;
static const char *g_group = "";

static void group(const char *name) {
    g_group = name;
    std::cout << "\n[" << name << "]" << std::endl;
}

static void fail(int line, const std::string &what) {
    g_failures++;
    std::cout << "  FAIL  " << g_group << " (line " << line << "): " << what << std::endl;
}

static void pass(const char *what) {
    std::cout << "  ok    " << what << std::endl;
}

static void checkTrue(int line, const char *expr, bool ok) {
    g_checks++;
    if (ok) {
        pass(expr);
        return;
    }
    fail(line, std::string(expr) + " is false");
}

template <class A, class B>
static void checkEq(int line, const char *expr, const A &actual, const B &expected) {
    g_checks++;
    if (actual == expected) {
        pass(expr);
        return;
    }
    std::ostringstream ss;
    ss << expr << ": got <" << actual << ">, want <" << expected << ">";
    fail(line, ss.str());
}

#define CHECK(expr)        checkTrue(__LINE__, #expr, bool(expr))
#define CHECK_EQ(a, b)     checkEq(__LINE__, #a, (a), (b))
// The error code, spelled so a mismatch prints both names as numbers.
#define CHECK_ERR(res, e)  checkEq(__LINE__, #res " == " #e, int((res).error()), int(e))

/// Captures whatever the library writes to stdout, which is where the help text goes.
class StdoutCapture {
public:
    StdoutCapture() {
        _path = (fs::temp_directory_path() / "syscmdline_test_capture.txt").string();
        std::fflush(stdout);
        _saved = SCL_DUP(SCL_FILENO(stdout));
        _file = std::fopen(_path.c_str(), "w+b"); // binary, so Windows leaves the newlines alone
        if (_file) {
            SCL_DUP2(SCL_FILENO(_file), SCL_FILENO(stdout));
        }
    }

    ~StdoutCapture() {
        stop();
    }

    std::string stop() {
        if (!_file) {
            return _text;
        }
        std::fflush(stdout);
        SCL_DUP2(_saved, SCL_FILENO(stdout));
#ifdef _WIN32
        _close(_saved);
#else
        close(_saved);
#endif
        std::fclose(_file);
        _file = nullptr;

        std::ifstream in(_path, std::ios::binary);
        _text.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        in.close();
        // Whichever way the console wrote them, the assertions below speak in "\n".
        _text.erase(std::remove(_text.begin(), _text.end(), '\r'), _text.end());
        std::error_code ec;
        fs::remove(_path, ec);
        return _text;
    }

private:
    std::string _path;
    std::string _text;
    std::FILE *_file = nullptr;
    int _saved = -1;
};

static bool contains(const std::string &haystack, const std::string &needle) {
    return haystack.find(needle) != std::string::npos;
}

/// The offset of `needle` in `haystack`, or -1. Used to assert on ordering.
static int indexOf(const std::string &haystack, const std::string &needle) {
    auto pos = haystack.find(needle);
    return pos == std::string::npos ? -1 : int(pos);
}

static std::string helpTextOf(const SCL::Parser &parser, const std::vector<std::string> &args) {
    StdoutCapture capture;
    SCL::Parser copy = parser;
    copy.parse(args).showHelpText();
    return capture.stop();
}

// ---------------------------------------------------------------------------
// Value
// ---------------------------------------------------------------------------

static void test_value() {
    group("Value");

    CHECK_EQ(int(SCL::Value().type()), int(SCL::Value::Null));
    CHECK_EQ(SCL::Value(42).toInt(), 42);
    CHECK_EQ(SCL::Value(int64_t(1) << 40).toInt64(), int64_t(1) << 40);
    CHECK_EQ(SCL::Value("text").toString(), std::string("text"));
    CHECK(SCL::Value(true).toBool());

    // Round trips through fromString, including the base prefixes.
    CHECK_EQ(SCL::Value::fromString("123", SCL::Value::Int).toInt(), 123);
    CHECK_EQ(SCL::Value::fromString("-123", SCL::Value::Int).toInt(), -123);
    CHECK_EQ(SCL::Value::fromString("+7", SCL::Value::Int).toInt(), 7);
    CHECK_EQ(SCL::Value::fromString("0x1F", SCL::Value::Int).toInt(), 31);
    CHECK_EQ(SCL::Value::fromString("0b101", SCL::Value::Int).toInt(), 5);
    CHECK_EQ(SCL::Value::fromString("0o17", SCL::Value::Int).toInt(), 15);
    CHECK_EQ(SCL::Value::fromString("true", SCL::Value::Bool).toBool(), true);
    CHECK_EQ(SCL::Value::fromString("FALSE", SCL::Value::Bool).toBool(), false);
    CHECK_EQ(SCL::Value::fromString("1.5", SCL::Value::Double).toDouble(), 1.5);

    // A token the type cannot represent gives Null rather than a zero.
    CHECK_EQ(int(SCL::Value::fromString("abc", SCL::Value::Int).type()), int(SCL::Value::Null));
    CHECK_EQ(int(SCL::Value::fromString("12x", SCL::Value::Int).type()), int(SCL::Value::Null));

    // An empty token used to read `front()` of an empty string.
    CHECK_EQ(int(SCL::Value::fromString("", SCL::Value::Int).type()), int(SCL::Value::Null));
    CHECK_EQ(int(SCL::Value::fromString("", SCL::Value::Double).type()), int(SCL::Value::Null));
    CHECK_EQ(int(SCL::Value::fromString("", SCL::Value::String).type()), int(SCL::Value::Null));

    CHECK(SCL::Value("a") == SCL::Value("a"));
    CHECK(SCL::Value("a") != SCL::Value("b"));
    CHECK(SCL::Value(1) != SCL::Value("1")); // different types never compare equal

    CHECK_EQ(std::string(SCL::Value::typeName(SCL::Value::Int)), std::string("int"));

    // Copy and move keep the string alive on both sides.
    SCL::Value a("hello");
    SCL::Value b = a;
    SCL::Value c = std::move(b);
    CHECK_EQ(a.toString(), std::string("hello"));
    CHECK_EQ(c.toString(), std::string("hello"));
}

// ---------------------------------------------------------------------------
// Value semantics of the symbols themselves
// ---------------------------------------------------------------------------

static void test_copy_semantics() {
    group("Copy semantics");

    // A copy that is then written to must not disturb the original, and neither
    // may outlive the other's storage.
    SCL::Argument arg("original");
    SCL::Argument copy = arg;
    copy.setName("changed");
    CHECK_EQ(arg.name(), std::string("original"));
    CHECK_EQ(copy.name(), std::string("changed"));

    // The written-to copy owns its data alone.
    CHECK(copy.isDetached());

    // A third reference taken from the copy, then dropped, leaves the copy intact.
    {
        SCL::Argument third = copy;
        CHECK_EQ(third.name(), std::string("changed"));
    }
    CHECK_EQ(copy.name(), std::string("changed"));

    // The same for a whole command tree.
    SCL::Command leaf("leaf", "leaf desc");
    SCL::Command mid("mid", "mid desc");
    mid.addCommand(leaf);
    SCL::Command root("root", "root desc");
    root.addCommand(mid);

    SCL::Command rootCopy = root;
    rootCopy.setName("other");
    CHECK_EQ(root.name(), std::string("root"));
    CHECK_EQ(rootCopy.name(), std::string("other"));
    CHECK_EQ(rootCopy.commandCount(), 1);
    CHECK_EQ(rootCopy.command(0).name(), std::string("mid"));
    CHECK_EQ(rootCopy.command(0).command(0).name(), std::string("leaf"));

    // Options are values too.
    SCL::Option opt("--flag", "desc");
    SCL::Option optCopy = opt;
    optCopy.setTokens({"--other"});
    CHECK_EQ(opt.token(), std::string("--flag"));
    CHECK_EQ(optCopy.token(), std::string("--other"));
}

// ---------------------------------------------------------------------------
// Positional arguments
// ---------------------------------------------------------------------------

static void test_positional_arguments() {
    group("Positional arguments");

    {
        SCL::Command cmd("cmd");
        cmd.addArguments({SCL::Argument("a"), SCL::Argument("b")});
        SCL::Parser parser(cmd);

        auto missing = parser.parse({"cmd", "1"});
        CHECK_ERR(missing, SCL::ParseResult::MissingCommandArgument);

        auto tooMany = parser.parse({"cmd", "1", "2", "3"});
        CHECK_ERR(tooMany, SCL::ParseResult::TooManyArguments);

        auto ok = parser.parse({"cmd", "1", "2"});
        CHECK_ERR(ok, SCL::ParseResult::NoError);
        CHECK_EQ(ok.value("a").toString(), std::string("1"));
        CHECK_EQ(ok.value("b").toString(), std::string("2"));
        CHECK_EQ(ok.value(0).toString(), std::string("1"));
        CHECK(ok.isArgumentSet("a"));
    }

    // An optional argument falls back to its default value.
    {
        SCL::Command cmd("cmd");
        cmd.addArguments({
            SCL::Argument("a"),
            SCL::Argument("b", "optional one", false, SCL::Value("fallback")),
        });
        SCL::Parser parser(cmd);

        auto given = parser.parse({"cmd", "1", "2"});
        CHECK_ERR(given, SCL::ParseResult::NoError);
        CHECK_EQ(given.value("b").toString(), std::string("2"));

        auto omitted = parser.parse({"cmd", "1"});
        CHECK_ERR(omitted, SCL::ParseResult::NoError);
        CHECK_EQ(omitted.value("b").toString(), std::string("fallback"));
        CHECK(!omitted.isArgumentSet("b"));
    }

    // A multi-value argument leaves room for what has to follow it.
    {
        SCL::Command cmd("cmd");
        SCL::Argument files("files");
        files.setNumber(SCL::Argument::MultiValue);
        cmd.addArguments({files, SCL::Argument("dest")});
        SCL::Parser parser(cmd);

        auto res = parser.parse({"cmd", "a", "b", "c", "out"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK_EQ(int(res.values("files").size()), 3);
        CHECK_EQ(res.values("files").front().toString(), std::string("a"));
        CHECK_EQ(res.values("files").back().toString(), std::string("c"));
        CHECK_EQ(res.value("dest").toString(), std::string("out"));

        auto minimal = parser.parse({"cmd", "a", "out"});
        CHECK_ERR(minimal, SCL::ParseResult::NoError);
        CHECK_EQ(int(minimal.values("files").size()), 1);
        CHECK_EQ(minimal.value("dest").toString(), std::string("out"));
    }

    // A remainder takes everything left over. An option this command declared is
    // still recognised as one, so it is the options that are pulled out first and
    // the remainder gets what is left rather than the whole tail verbatim.
    {
        SCL::Command cmd("cmd");
        SCL::Argument rest("rest");
        rest.setNumber(SCL::Argument::Remainder);
        cmd.addArguments({SCL::Argument("first"), rest});
        cmd.addOption(SCL::Option("--flag", "a flag"));
        SCL::Parser parser(cmd);

        auto res = parser.parse({"cmd", "one", "two", "--flag", "three"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK_EQ(res.value("first").toString(), std::string("one"));
        CHECK_EQ(int(res.values("rest").size()), 2);
        CHECK_EQ(res.values("rest")[0].toString(), std::string("two"));
        CHECK_EQ(res.values("rest")[1].toString(), std::string("three"));
        CHECK(res.isOptionSet("--flag"));
    }

    // An argument named by an index nobody declared reads as empty rather than crashing.
    {
        SCL::Command cmd("cmd");
        cmd.addArgument(SCL::Argument("a"));
        SCL::Parser parser(cmd);
        auto res = parser.parse({"cmd", "1"});
        CHECK_EQ(res.indexOfArgument("nope"), -1);
        CHECK(res.values(-1).empty());
        CHECK(res.values(99).empty());
        CHECK(res.value("nope").toString().empty());
    }
}

// ---------------------------------------------------------------------------
// Expected values and validators
// ---------------------------------------------------------------------------

static void test_expected_and_validator() {
    group("Expected values and validators");

    {
        SCL::Argument arg("mode");
        arg.setExpectedValues({"fast", "slow"});
        SCL::Command cmd("cmd");
        cmd.addArgument(arg);
        SCL::Parser parser(cmd);

        CHECK_ERR(parser.parse({"cmd", "fast"}), SCL::ParseResult::NoError);
        CHECK_ERR(parser.parse({"cmd", "other"}), SCL::ParseResult::InvalidArgumentValue);
    }

    // A default value declares the type, and a token that is not of it is rejected.
    {
        SCL::Command cmd("cmd");
        cmd.addArgument(SCL::Argument("n", "a number", false, SCL::Value(10)));
        SCL::Parser parser(cmd);

        auto ok = parser.parse({"cmd", "42"});
        CHECK_ERR(ok, SCL::ParseResult::NoError);
        CHECK_EQ(ok.value("n").toInt(), 42);
        CHECK_ERR(parser.parse({"cmd", "text"}), SCL::ParseResult::ArgumentTypeMismatch);
        CHECK_EQ(parser.parse({"cmd"}).value("n").toInt(), 10);
    }

    {
        SCL::Argument arg("even");
        arg.setValidator([](const std::string &token, SCL::Value *out, std::string *err) {
            try {
                int n = std::stoi(token);
                if (n % 2 != 0) {
                    *err = "not even";
                    return false;
                }
                *out = SCL::Value(n);
                return true;
            } catch (...) {
                *err = "not a number";
                return false;
            }
        });
        SCL::Command cmd("cmd");
        cmd.addArgument(arg);
        SCL::Parser parser(cmd);

        auto ok = parser.parse({"cmd", "4"});
        CHECK_ERR(ok, SCL::ParseResult::NoError);
        CHECK_EQ(ok.value("even").toInt(), 4);

        auto bad = parser.parse({"cmd", "5"});
        CHECK_ERR(bad, SCL::ParseResult::ArgumentValidateFailed);
        CHECK(contains(bad.errorText(), "not even"));
    }
}

// ---------------------------------------------------------------------------
// Options
// ---------------------------------------------------------------------------

static void test_options() {
    group("Options");

    {
        SCL::Command cmd("cmd");
        cmd.addOptions({
            SCL::Option({"-a", "--all"}, "a flag"),
            SCL::Option({"-o", "--out"}, "with a value").arg("dir"),
        });
        SCL::Parser parser(cmd);

        auto res = parser.parse({"cmd", "-a", "--out", "build"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK(res.isOptionSet("-a"));
        CHECK(res.isOptionSet("--all")); // either spelling answers
        CHECK(res.isOptionSet("--out"));
        CHECK_EQ(res.valueForOption("-o").toString(), std::string("build"));

        auto absent = parser.parse({"cmd"});
        CHECK(!absent.isOptionSet("-a"));
        CHECK_EQ(absent.option("-a").count(), 0);

        CHECK_ERR(parser.parse({"cmd", "--nope"}), SCL::ParseResult::UnknownOption);
        CHECK_ERR(parser.parse({"cmd", "--out"}), SCL::ParseResult::MissingOptionArgument);
    }

    // A repeatable option, and the occurrence limit on one that is not.
    {
        SCL::Command cmd("cmd");
        cmd.addOptions({
            SCL::Option({"-e", "--exclude"}, "repeatable").arg("pattern").multi(),
            SCL::Option({"-p", "--project"}, "once only").arg("name"),
        });
        SCL::Parser parser(cmd);

        auto res = parser.parse({"cmd", "-e", "one", "-e", "two", "-e", "three"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK_EQ(res.option("-e").count(), 3);
        CHECK_EQ(int(res.option("-e").allValues().size()), 3);
        CHECK_EQ(res.option("-e").value(0, 1).toString(), std::string("two"));

        CHECK_ERR(parser.parse({"cmd", "-p", "a", "-p", "b"}),
                  SCL::ParseResult::OptionOccurTooMuch);
    }

    // Two arguments on one option, read by (argument index, occurrence).
    {
        SCL::Command cmd("cmd");
        cmd.addOption(SCL::Option({"-i", "--include"}, "pattern and subdir")
                          .arg("regex")
                          .arg("subdir")
                          .multi());
        SCL::Parser parser(cmd);

        auto res = parser.parse({"cmd", "-i", "r1", "s1", "-i", "r2", "s2"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK_EQ(res.option("-i").count(), 2);
        CHECK_EQ(res.option("-i").value(0, 0).toString(), std::string("r1"));
        CHECK_EQ(res.option("-i").value(1, 0).toString(), std::string("s1"));
        CHECK_EQ(res.option("-i").value(0, 1).toString(), std::string("r2"));
        CHECK_EQ(res.option("-i").value(1, 1).toString(), std::string("s2"));
    }

    // A required option that was not given.
    {
        SCL::Command cmd("cmd");
        cmd.addOption(SCL::Option({"-r", "--required"}, "must be given").required());
        SCL::Parser parser(cmd);
        CHECK_ERR(parser.parse({"cmd"}), SCL::ParseResult::MissingRequiredOption);
        CHECK_ERR(parser.parse({"cmd", "-r"}), SCL::ParseResult::NoError);
    }

    // Reading an option nobody declared answers empty rather than crashing.
    {
        SCL::Command cmd("cmd");
        cmd.addOption(SCL::Option("--real", "declared"));
        SCL::Parser parser(cmd);
        auto res = parser.parse({"cmd"});
        CHECK_EQ(res.indexOfOption("--ghost"), -1);
        CHECK(!res.isOptionSet("--ghost"));
        CHECK_EQ(res.option("--ghost").count(), 0);
        CHECK(res.valueForOption("--ghost").toString().empty());
    }
}

static void test_short_match_and_flags() {
    group("Short match and grouped flags");

    // -DKEY=VALUE as one token.
    {
        SCL::Command cmd("cmd");
        cmd.addOption(SCL::Option({"-D", "--define"}, "a definition")
                          .arg("expr")
                          .multi()
                          .short_match(SCL::Option::ShortMatchSingleChar));
        SCL::Parser parser(cmd);

        auto joined = parser.parse({"cmd", "-DKEY=VALUE"});
        CHECK_ERR(joined, SCL::ParseResult::NoError);
        CHECK_EQ(joined.valueForOption("-D").toString(), std::string("KEY=VALUE"));

        auto separate = parser.parse({"cmd", "-D", "KEY=VALUE"});
        CHECK_ERR(separate, SCL::ParseResult::NoError);
        CHECK_EQ(separate.valueForOption("-D").toString(), std::string("KEY=VALUE"));

        auto equals = parser.parse({"cmd", "--define=KEY=VALUE"});
        CHECK_ERR(equals, SCL::ParseResult::NoError);
        CHECK_EQ(equals.valueForOption("-D").toString(), std::string("KEY=VALUE"));
    }

    // -abc meaning -a -b -c, only when asked for.
    {
        SCL::Command cmd("cmd");
        cmd.addOptions({
            SCL::Option("-a", "flag a"),
            SCL::Option("-b", "flag b"),
            SCL::Option("-c", "flag c"),
        });
        SCL::Parser parser(cmd);

        auto grouped = parser.parse({"cmd", "-abc"}, SCL::Parser::AllowUnixGroupFlags);
        CHECK_ERR(grouped, SCL::ParseResult::NoError);
        CHECK(grouped.isOptionSet("-a"));
        CHECK(grouped.isOptionSet("-b"));
        CHECK(grouped.isOptionSet("-c"));

        CHECK_ERR(parser.parse({"cmd", "-abc"}), SCL::ParseResult::UnknownOption);
    }

    // Dos spelling, and case folding, each only when asked for.
    {
        SCL::Command cmd("cmd");
        cmd.addOption(SCL::Option("/Q", "quiet"));
        SCL::Parser parser(cmd);
        CHECK(parser.parse({"cmd", "/Q"}).isOptionSet("/Q"));
        CHECK(parser.parse({"cmd", "/q"}, SCL::Parser::IgnoreOptionCase).isOptionSet("/Q"));
        // Without the flag it is not an option, and since it does not begin with a
        // dash it is reported as a positional the command cannot take.
        auto wrongCase = parser.parse({"cmd", "/q"});
        CHECK_ERR(wrongCase, SCL::ParseResult::TooManyArguments);
        CHECK(!wrongCase.isOptionSet("/Q"));
    }
}

// ---------------------------------------------------------------------------
// Subcommands
// ---------------------------------------------------------------------------

static void test_subcommands() {
    group("Subcommands");

    SCL::Command leaf("leaf", "the deepest one");
    leaf.addArgument(SCL::Argument("x"));

    SCL::Command mid("mid", "the middle one");
    mid.addCommand(leaf);

    SCL::Command other("other", "a sibling of mid");

    SCL::Command root("root", "the root");
    root.addCommands({other, mid});

    SCL::Parser parser(root);

    // One level down.
    {
        auto res = parser.parse({"root", "other"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK_EQ(res.command().name(), std::string("other"));
        CHECK_EQ(int(res.commandIndexStack().size()), 1);
    }

    // Two levels down, reached through the second child rather than the first.
    {
        auto res = parser.parse({"root", "mid", "leaf", "value"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK_EQ(res.command().name(), std::string("leaf"));
        CHECK_EQ(int(res.commandIndexStack().size()), 2);
        CHECK_EQ(res.value("x").toString(), std::string("value"));
    }

    // An unknown name where a subcommand was expected.
    CHECK_ERR(parser.parse({"root", "nope"}), SCL::ParseResult::UnknownCommand);

    // Case folding on command names, only when asked for.
    CHECK_EQ(parser.parse({"root", "MID", "leaf", "v"}, SCL::Parser::IgnoreCommandCase)
                 .command()
                 .name(),
             std::string("leaf"));
}

static void test_global_options() {
    group("Global options");

    // A global option declared at a level must reach every level below it, and
    // the collection must follow the path actually taken rather than the first
    // child at each step.
    SCL::Command leaf("leaf", "leaf");
    leaf.addArgument(SCL::Argument("x"));

    SCL::Command mid("mid", "mid");
    SCL::Option midGlobal("--mid-global", "declared on mid");
    midGlobal.setGlobal(true);
    mid.addOption(midGlobal);
    mid.addCommand(leaf);

    // A sibling in front of `mid` with a different number of global options, so
    // that walking the wrong branch is visible rather than harmless.
    SCL::Command decoy("decoy", "decoy");
    SCL::Option decoyGlobalA("--decoy-a", "declared on decoy");
    SCL::Option decoyGlobalB("--decoy-b", "declared on decoy");
    decoyGlobalA.setGlobal(true);
    decoyGlobalB.setGlobal(true);
    decoy.addOptions({decoyGlobalA, decoyGlobalB});
    decoy.addCommand(SCL::Command("dleaf", "dleaf"));

    SCL::Command root("root", "root");
    SCL::Option rootGlobal("--root-global", "declared on root");
    rootGlobal.setGlobal(true);
    root.addOption(rootGlobal);
    root.addCommands({decoy, mid});

    SCL::Parser parser(root);

    {
        auto res = parser.parse({"root", "mid", "leaf", "v", "--root-global", "--mid-global"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK(res.isOptionSet("--root-global"));
        CHECK(res.isOptionSet("--mid-global"));
        CHECK_EQ(int(res.globalOptions().size()), 2);
    }

    // The decoy branch sees its own, not mid's.
    {
        auto res = parser.parse({"root", "decoy", "dleaf", "--decoy-a"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK(res.isOptionSet("--decoy-a"));
        CHECK_EQ(int(res.globalOptions().size()), 3); // root's one plus decoy's two
    }

    // A non-global option of a parent is not in scope for the child.
    {
        SCL::Command child("child", "child");
        SCL::Command parent("parent", "parent");
        parent.addOption(SCL::Option("--local", "not global"));
        parent.addCommand(child);
        CHECK_ERR(SCL::Parser(parent).parse({"parent", "child", "--local"}),
                  SCL::ParseResult::UnknownOption);
    }
}

// ---------------------------------------------------------------------------
// Prior levels and exclusive groups
// ---------------------------------------------------------------------------

static void test_prior_and_exclusive() {
    group("Prior levels and exclusive options");

    // --help must be answerable on a line that is missing a required argument.
    {
        SCL::Command cmd("cmd");
        cmd.addArgument(SCL::Argument("required-one"));
        cmd.addHelpOption();
        SCL::Parser parser(cmd);

        auto res = parser.parse({"cmd", "--help"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK(res.isHelpSet());
        CHECK_ERR(parser.parse({"cmd"}), SCL::ParseResult::MissingCommandArgument);
    }

    // showHelpIfNoArg sets the option on an otherwise empty line.
    {
        SCL::Command cmd("cmd");
        cmd.addArgument(SCL::Argument("required-one"));
        cmd.addHelpOption(true);
        auto res = SCL::Parser(cmd).parse({"cmd"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK(res.isHelpSet());
    }

    // A version option, and the role lookup that goes with it.
    {
        SCL::Command cmd("cmd");
        cmd.addVersionOption("1.2.3");
        SCL::Parser parser(cmd);
        auto res = parser.parse({"cmd", "--version"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK(res.isVersionSet());
        CHECK(!res.isHelpSet());
        CHECK_EQ(res.command().versionString(), std::string("1.2.3"));
    }

    // Two options in one exclusive group cannot both be given.
    {
        SCL::Command cmd("cmd");
        cmd.addOptions({SCL::Option("--opt1", "one"), SCL::Option("--opt2", "two")}, "group");
        SCL::Parser parser(cmd);
        CHECK_ERR(parser.parse({"cmd", "--opt1"}), SCL::ParseResult::NoError);
        CHECK_ERR(parser.parse({"cmd", "--opt2"}), SCL::ParseResult::NoError);
        CHECK_ERR(parser.parse({"cmd", "--opt1", "--opt2"}),
                  SCL::ParseResult::MutuallyExclusiveOptions);
    }
}

// ---------------------------------------------------------------------------
// Catalogue and help text
// ---------------------------------------------------------------------------

static void test_catalogue() {
    group("Catalogue");

    // Two calls naming the same heading must both land under it.
    {
        SCL::CommandCatalogue cc;
        cc.addCommands("Group A", {"first"});
        cc.addCommands("Group A", {"second"}); // same heading, second call
        cc.addCommands("Group B", {"third"});

        SCL::Command root("root", "root");
        root.addCommands({
            SCL::Command("first", "the first"),
            SCL::Command("second", "the second"),
            SCL::Command("third", "the third"),
            SCL::Command("loose", "not catalogued"),
        });
        root.setCatalogue(cc);
        root.addHelpOption();

        auto text = helpTextOf(SCL::Parser(root), {"root", "--help"});
        CHECK(contains(text, "Group A:"));
        CHECK(contains(text, "Group B:"));

        // Both members of Group A belong to it, so both come before Group B's heading.
        CHECK(indexOf(text, "first") > indexOf(text, "Group A:"));
        CHECK(indexOf(text, "second") > indexOf(text, "Group A:"));
        CHECK(indexOf(text, "second") < indexOf(text, "Group B:"));

        // What no group claimed still shows up, under the default heading.
        CHECK(contains(text, "loose"));
        CHECK(indexOf(text, "loose") > indexOf(text, "Group B:"));
    }

    // A catalogued option must be the option that was named, which it is not when
    // the lookup and the listing disagree about what an index means. Global
    // options make the two spaces differ.
    {
        SCL::Option global("--global-one", "a global option");
        global.setGlobal(true);

        SCL::CommandCatalogue cc;
        cc.addOptions("Chosen", {"--bb"});

        SCL::Command sub("sub", "a subcommand");
        sub.addOptions({
            SCL::Option("--aa", "description of aa"),
            SCL::Option("--bb", "description of bb"),
        });
        sub.setCatalogue(cc);
        sub.addHelpOption();

        SCL::Command root("root", "root");
        root.addOption(global);
        root.addCommand(sub);

        auto text = helpTextOf(SCL::Parser(root), {"root", "sub", "--help"});
        CHECK(contains(text, "Chosen:"));

        // The chosen heading names --bb and its description, not somebody else's.
        auto chosen = text.substr(size_t(indexOf(text, "Chosen:")));
        chosen = chosen.substr(0, chosen.find("\n\n"));
        CHECK(contains(chosen, "--bb"));
        CHECK(contains(chosen, "description of bb"));
        CHECK(!contains(chosen, "--global-one"));
        CHECK(!contains(chosen, "--aa"));

        // And --bb is not also listed again under the default heading.
        auto rest = text.substr(size_t(indexOf(text, "Options:")));
        CHECK(!contains(rest, "--bb"));
        CHECK(contains(rest, "--aa"));
        CHECK(contains(rest, "--global-one"));
    }
}

static void test_help_text() {
    group("Help text");

    // The usage line names the whole path down to the command that was reached.
    {
        SCL::Command leaf("leaf", "leaf desc");
        leaf.addHelpOption();
        SCL::Command mid("mid", "mid desc");
        mid.addCommand(leaf);
        SCL::Command root("root", "root desc");
        root.addCommand(mid);

        auto text = helpTextOf(SCL::Parser(root), {"root", "mid", "leaf", "--help"});
        CHECK(contains(text, "root mid leaf"));
    }

    // Arguments, options and the description all reach the page.
    {
        SCL::Command cmd("cmd", "what the command does");
        SCL::Argument files("files", "the source files");
        files.setNumber(SCL::Argument::MultiValue);
        cmd.addArguments({files, SCL::Argument("dest", "where they go")});
        cmd.addOption(SCL::Option({"-f", "--force"}, "overwrite what is there"));
        cmd.addHelpOption();

        SCL::Parser parser(cmd);
        parser.setPrologue("the prologue");
        parser.setEpilogue("the epilogue");

        auto text = helpTextOf(parser, {"cmd", "--help"});
        CHECK(contains(text, "the prologue"));
        CHECK(contains(text, "what the command does"));
        CHECK(contains(text, "cmd <files>... <dest>"));
        CHECK(contains(text, "the source files"));
        CHECK(contains(text, "-f, --force"));
        CHECK(contains(text, "overwrite what is there"));
        CHECK(contains(text, "the epilogue"));

        // In the fixed order: description, usage, arguments, options, epilogue.
        CHECK(indexOf(text, "the prologue") < indexOf(text, "Usage:"));
        CHECK(indexOf(text, "Usage:") < indexOf(text, "Arguments:"));
        CHECK(indexOf(text, "Arguments:") < indexOf(text, "Options:"));
        CHECK(indexOf(text, "Options:") < indexOf(text, "the epilogue"));
    }

    // The extras only show when they are asked for.
    {
        SCL::Argument mode("mode", "how to run", false, SCL::Value("fast"));
        mode.setExpectedValues({"fast", "slow"});
        SCL::Command cmd("cmd", "desc");
        cmd.addArgument(mode);
        cmd.addHelpOption();

        SCL::Parser plain(cmd);
        auto plainText = helpTextOf(plain, {"cmd", "--help"});
        CHECK(!contains(plainText, "Default"));

        SCL::Parser verbose(cmd);
        verbose.setDisplayOptions(SCL::Parser::ShowArgumentDefaultValue |
                                  SCL::Parser::ShowArgumentExpectedValues);
        auto verboseText = helpTextOf(verbose, {"cmd", "--help"});
        CHECK(contains(verboseText, "Default: fast"));
        CHECK(contains(verboseText, "\"slow\""));
    }
}

// ---------------------------------------------------------------------------
// Response files
// ---------------------------------------------------------------------------

static void test_response_file() {
    group("Response file");

    auto path = fs::temp_directory_path() / "syscmdline_test_response.txt";
    {
        std::ofstream out(path, std::ios::binary);
        out << "alpha\n";
        out << "\n";           // a blank line, which trimming used to walk off
        out << "  beta  \n";   // surrounding space is dropped
        out << "\"gamma one\"\n"; // quotes are stripped, so a space survives
        out << "--flag\n";
    }

    SCL::Command cmd("cmd");
    SCL::Argument words("words");
    words.setNumber(SCL::Argument::MultiValue);
    cmd.addArgument(words);
    cmd.addOption(SCL::Option("--flag", "a flag"));

    SCL::Parser parser(cmd);
    auto res = parser.parse({"cmd", "@" + path.string()}, SCL::Parser::EnableResponseFile);
    CHECK_ERR(res, SCL::ParseResult::NoError);
    CHECK_EQ(int(res.values("words").size()), 3);
    CHECK_EQ(res.values("words")[0].toString(), std::string("alpha"));
    CHECK_EQ(res.values("words")[1].toString(), std::string("beta"));
    CHECK_EQ(res.values("words")[2].toString(), std::string("gamma one"));
    CHECK(res.isOptionSet("--flag"));

    // A file that is not there is an error rather than a crash.
    auto missing = parser.parse({"cmd", "@no_such_response_file_here.txt"},
                                SCL::Parser::EnableResponseFile);
    CHECK_ERR(missing, SCL::ParseResult::ErrorReadingResponseFile);

    // The tokens a response file brings in may name a subcommand, which means
    // the search for one has to run again over them.
    auto nestedPath = fs::temp_directory_path() / "syscmdline_test_response_sub.txt";
    {
        std::ofstream out(nestedPath, std::ios::binary);
        out << "sub\n";
        out << "value\n";
        out << "--flag\n";
    }

    SCL::Command sub("sub", "a subcommand");
    sub.addArgument(SCL::Argument("x"));
    sub.addOption(SCL::Option("--flag", "a flag"));
    SCL::Command root("root", "root");
    root.addCommand(sub);

    SCL::Parser rootParser(root);
    auto nested =
        rootParser.parse({"root", "@" + nestedPath.string()}, SCL::Parser::EnableResponseFile);
    CHECK_ERR(nested, SCL::ParseResult::NoError);
    CHECK_EQ(nested.command().name(), std::string("sub"));
    CHECK_EQ(nested.value("x").toString(), std::string("value"));
    CHECK(nested.isOptionSet("--flag"));

    // A file named after a subcommand on the command line still works.
    auto after = rootParser.parse({"root", "sub", "@" + path.string()},
                                  SCL::Parser::EnableResponseFile);
    CHECK_EQ(after.command().name(), std::string("sub"));

    std::error_code ec;
    fs::remove(path, ec);
    fs::remove(nestedPath, ec);
}

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------

static void test_invoke() {
    group("Invoke");

    int reached = 0;

    SCL::Command sub("sub", "a subcommand");
    // The default value is what declares the type. Without one the token stays a
    // string, and reading it as an int gives zero rather than the number.
    sub.addArgument(SCL::Argument("x", "a number", true, SCL::Value(0)));
    sub.setHandler([&reached](const SCL::ParseResult &result) -> int {
        reached++;
        return result.value("x").toInt();
    });

    SCL::Command root("root", "root");
    root.addCommand(sub);
    root.setHandler([](const SCL::ParseResult &) -> int { return 100; });

    SCL::Parser parser(root);
    CHECK_EQ(parser.invoke({"root", "sub", "7"}), 7);
    CHECK_EQ(reached, 1);
    CHECK_EQ(parser.invoke({"root"}), 100);
    CHECK_EQ(reached, 1);

    // A line that does not parse returns the error code and leaves the handler alone.
    {
        StdoutCapture capture; // the error report goes to the console
        int code = parser.invoke({"root", "sub"}, 42);
        capture.stop();
        CHECK_EQ(code, 42);
        CHECK_EQ(reached, 1);
    }
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

static void test_edge_cases() {
    group("Edge cases");

    // An empty token is a value, not a crash.
    {
        SCL::Command cmd("cmd");
        cmd.addArgument(SCL::Argument("x"));
        auto res = SCL::Parser(cmd).parse({"cmd", ""});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK(res.value("x").toString().empty());
    }

    // A command line with nothing on it at all.
    {
        SCL::Command cmd("cmd");
        CHECK_ERR(SCL::Parser(cmd).parse({"cmd"}), SCL::ParseResult::NoError);
    }

    // A value that begins with a dash but is not a declared option.
    {
        SCL::Command cmd("cmd");
        SCL::Argument rest("rest");
        rest.setNumber(SCL::Argument::Remainder);
        cmd.addArgument(rest);
        auto res = SCL::Parser(cmd).parse({"cmd", "--anything", "-x"});
        CHECK_ERR(res, SCL::ParseResult::NoError);
        CHECK_EQ(int(res.values("rest").size()), 2);
    }

    // A token that is one option's spelling with more after it, where no option
    // takes a joined value.
    {
        SCL::Command cmd("cmd");
        cmd.addOption(SCL::Option("-a", "a flag"));
        CHECK_ERR(SCL::Parser(cmd).parse({"cmd", "-abc"}), SCL::ParseResult::UnknownOption);
    }

    // Reading a result that failed answers rather than crashing.
    {
        SCL::Command cmd("cmd");
        cmd.addArgument(SCL::Argument("x"));
        auto res = SCL::Parser(cmd).parse({"cmd"});
        CHECK(res.error() != SCL::ParseResult::NoError);
        CHECK(!res.errorText().empty());
        CHECK(res.isValid()); // it has data, it just says what went wrong
        CHECK_EQ(int(res.arguments().size()), 1);
    }
}

// ---------------------------------------------------------------------------

int main(int argc, char *argv[]) {
    SYSCMDLINE_UNUSED(argc);
    SYSCMDLINE_UNUSED(argv);

    std::cout << "SysCmdLine regression tests" << std::endl;

    test_value();
    test_copy_semantics();
    test_positional_arguments();
    test_expected_and_validator();
    test_options();
    test_short_match_and_flags();
    test_subcommands();
    test_global_options();
    test_prior_and_exclusive();
    test_catalogue();
    test_help_text();
    test_response_file();
    test_invoke();
    test_edge_cases();

    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << g_checks << " checks, " << g_failures << " failed" << std::endl;
    return g_failures == 0 ? 0 : 1;
}
