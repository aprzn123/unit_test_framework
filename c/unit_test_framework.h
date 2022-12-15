// __GNUC__ required for __attribute__((constructor))
#ifdef __GNUC__

#ifndef TEST_COUNT
#define TEST_COUNT 256
#endif

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// Create a funcion named after the test name, and create a function that
// runs before main and adds a pointer to the test function to an array
// of functions
#define TEST(NAME)                                                      \
    static void test_##NAME(TestResult*);                               \
    static TestResult NAME(void) {                                      \
        TestResult out = {.successful=true};                            \
        test_##NAME(&out);                                              \
        return out;                                                     \
    }                                                                   \
    static void __attribute__((constructor)) register_##NAME(void) {    \
        tests[num_tests] = (NamedTest) {.fn = *NAME,                    \
                                        .name = #NAME,                  \
                                        .to_run = false};               \
        num_tests++;                                                    \
    }                                                                   \
    static void test_##NAME(TestResult* utf_secret_result) 

// Simple wrapper to match the style of the original
#define TEST_MAIN()                                                     \
    int main(int argc, char* argv[]) {                                  \
        return run_tests(argc, argv);                                   \
    }

#define FAIL(ERR) utf_secret_result->successful = false;                \
                  utf_secret_result->message = ERR;

#define ASSERT_TRUE(BOOL) if (assert_condition((BOOL), utf_secret_result, __LINE__, "ASSERT_TRUE(" #BOOL ")")) return;
#define ASSERT_FALSE(BOOL) if (assert_condition(!(BOOL), utf_secret_result, __LINE__, "ASSERT_FALSE(" #BOOL ")")) return;
#define ASSERT_EQUAL(A, B) if (assert_condition((A) == (B), utf_secret_result, __LINE__, "ASSERT_EQUAL(" #A ", " #B ")")) return;
#define ASSERT_NOT_EQUAL(A, B) if (assert_condition((A) != (B), utf_secret_result, __LINE__, "ASSERT_NOT_EQUAL(" #A ", " #B ")")) return;
#define ASSERT_ALMOST_EQUAL(A, B, EPS) if (assert_condition((A) - (B) < (EPS) && (B) - (A) < (EPS), utf_secret_result, __LINE__, \
        "ASSERT_ALMOST_EQUAL(" #A ", " #B ", " #EPS ")")) return;


#define UTF_COL_BLUE  "\033[34m"
#define UTF_COL_GREEN "\033[32;1m"
#define UTF_COL_RED   "\033[31;1m"
#define UTF_COL_RESET "\033[0m"


typedef struct TestResult {
    bool successful;
    char* message;
    int line_num;
} TestResult;

typedef TestResult (TestFunc)();

typedef struct NamedTest {
    TestFunc* fn; 
    char* name;
    bool to_run;
} NamedTest;


static NamedTest tests[TEST_COUNT];
static int num_tests = 0;


int help(char*);
int test_names();
bool enable_test(char*);

bool run_test(NamedTest *test, bool quiet) {
    if (!quiet) printf("%s...", test->name);
    TestResult out = test->fn();
    if (out.successful) {
        if (!quiet) printf(UTF_COL_GREEN "PASS\n" UTF_COL_RESET);
        return true;
    }
    if (quiet) printf("%s...", test->name);
    printf(UTF_COL_RED "FAIL" UTF_COL_RESET UTF_COL_BLUE "\nLine %i: %s\n" UTF_COL_RESET, out.line_num, out.message);
    return false;
}

int run_tests(int argc, char *argv[]) {
    int pass = 0, fail = 0;
    bool quiet = false;
    bool run_all_tests = true;

    // parse args
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) return help(argv[0]);
        else if (!strcmp(argv[i], "-q") || !strcmp(argv[i], "--quiet")) quiet = true;
        else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--show_test_names")) return test_names();
        else {
            run_all_tests = false;
            if (!enable_test(argv[i])) {
                printf("%s is not a test\n", argv[i]);
                return -1;
            }
        }
    }

    // run tests
    printf("--- TESTS ---\n");
    for (int i = 0; i < num_tests; i++) {
        if (!run_all_tests && !tests[i].to_run) continue;
        if (run_test(&tests[i], quiet)) {
            pass++;
        } else {
            fail++;
        }
    }
    printf("%i tests passed, %i tests failed", pass, fail);
    return 0;
}

bool assert_condition(bool condition, TestResult* result, int line, char* failure_name) {
    if (!condition) {
        result->message = failure_name;
        result->line_num = line;
        result->successful = false;
    }
    return !condition;
}

bool enable_test(char* name) {
    for (int i = 0; i < num_tests; ++i) {
        if (!strcmp(name, tests[i].name)) {
            tests[i].to_run = true;
            return true;
        }
    }
    return false;
}

int help(char* call) {
    printf("usage: %s [-h] [-n] [-q] [[TEST_NAME] ...]\n"
           "optional arguments:\n"
           " -h, --help\t\t show this help message and exit\n"
           " -n, --show_test_names\t print the names of all "
           "discovered test cases and exit\n"
           " -q, --quiet\t\t print a reduced summary of test"
           "results (only show tests that fail)\n"
           " TEST_NAME ...\t\t run only the test cases whose names "
           "are "
           "listed here. Note: If no test names are specified, all "
           "discovered tests are run by default.\n", call
    );
    return 0;
}

int test_names() {
    for (int i = 0; i < num_tests; i++) {
        printf("%s\n", tests[i].name);
    }
    return 0;
}

#endif
