#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#define ALPHABET_SIZE CHAR_MAX

#define MAX(x,y) ((x) > (y) ? (x) : (y))

static int find_occurrence(const char *haystack, const size_t comparison_start,
                           const char *needle, const size_t needle_len,
                           const char preceding)
{
    for (int i = comparison_start - 1; i >= 0; i--) {
        int match = strncmp(haystack + i, needle, needle_len);
        if (match == 0) {
            if (i == 0 || haystack[i - 1] != preceding) {
                return i;
            }
        }
    }
    return -1;
}

static int is_prefix(const char *str, const char *prefix,
                     const uint32_t prefix_len)
{
    uint32_t i = 0;

    /* Prefix must be a proper prefix, so if str and prefix are the same then
     * start comparing from the first character. */
    if (str == prefix) {
        i = 1;
    }

    for (; i < prefix_len; i++) {
        int match = strncmp(str, prefix + i, prefix_len - i);
        if (match == 0) {
            return i;
        }
    }

    return -1;
}

static void build_good_suffix_case1(uint32_t *good_suffix_array, const char *pattern,
                                    const size_t pattern_len)
{
    /* Case 1: matched string occurs in the pattern. */
    for (uint32_t i = 1; i < pattern_len; i++) {
        uint32_t matched_start_idx = pattern_len - i;
        int m = find_occurrence(pattern, matched_start_idx,
                                pattern + matched_start_idx, i,
                                pattern[matched_start_idx - 1]);
        if (m >= 0) {
            good_suffix_array[pattern_len - i] = matched_start_idx - m;
        } else {
            good_suffix_array[pattern_len - i] = 0;
        }
    }
}

static void build_good_suffix_case2(uint32_t *good_suffix_array, const char *pattern,
                                    const size_t pattern_len)
{
    /* Case 2: suffix of a matched string is a prefix of the pattern. */
    for (uint32_t i = 0; i < pattern_len; i++) {
        if (good_suffix_array[i] != 0) {
            /* If case 2 is applies only if case 1 didn't. */
            continue;
        }

        int m = is_prefix(pattern, pattern + i, pattern_len - i);
        if (m > 0) {
            good_suffix_array[i] = i + m;
        } else {
            good_suffix_array[i] = pattern_len;
        }
    }
}

void build_good_suffix_array(uint32_t *good_suffix_array, const char *pattern,
                             const size_t pattern_len)
{
    /* If the mismatch happens at the first character, we can only move by
     * one. */
    good_suffix_array[pattern_len] = 1;
    /* Case 1 cannot apply on the first element, so initialize it to zero. */
    good_suffix_array[0] = 0;

    build_good_suffix_case1(good_suffix_array, pattern, pattern_len);
    build_good_suffix_case2(good_suffix_array, pattern, pattern_len);
}

void build_bad_char_array(uint32_t *bad_char_array, const char *pattern,
                          const size_t pattern_len)
{
    for (uint32_t i = 0; i < ALPHABET_SIZE; i++) {
        bad_char_array[i] = pattern_len;
    }

    for (uint32_t i = 0; i < pattern_len; i++) {
        bad_char_array[pattern[i]] = pattern_len - i - 1;
    }
}

uint32_t boyer_moore(const char *text, const char *pattern)
{
    uint32_t matches = 0;

    const size_t pattern_len = strlen(pattern);
    const size_t text_len = strlen(text);

    uint32_t bad_char[ALPHABET_SIZE];
    uint32_t good_suffix[pattern_len + 1];

    build_bad_char_array(bad_char, pattern, pattern_len);
    build_good_suffix_array(good_suffix, pattern, pattern_len);

    for (uint32_t i = pattern_len - 1; i < text_len;)
    {
        uint32_t j = pattern_len - 1;
        while (text[i] == pattern[j]) {
            if (j == 0) {
                printf("match %u at %d\n", matches, i);
                matches++;
                break;
            }
            i--;
            j--;
        }

        const uint32_t matched_chars = pattern_len - 1 - j;
        const uint32_t delta1 = bad_char[text[i]] <= matched_chars ?
            1 : bad_char[text[i]] - matched_chars;
        const uint32_t delta2 = good_suffix[j + 1];

        const uint32_t shift = MAX(delta1, delta2);

        i += matched_chars + shift;
    }

    return matches;
}

/**
 * Tests start here. Typically you would have these in separate files but for
 * simpilicity, I dumped everything here.
 */

#define assert_equal_array(got, expected, size)                         \
    do {                                                                \
        for (size_t i = 0; i < (size); i++) {                           \
            if ((got)[i] != (expected)[i]) {                            \
                printf("%s:%d: %s: expected[%zu]=%d, got[%zu]=%d!\n",   \
                       __FILE__, __LINE__, __func__,                    \
                       i, (expected)[i], i, (got)[i]);                  \
                printf("%s:%d: %s: got:\n",                             \
                       __FILE__, __LINE__, __func__);                   \
                for (i = 0; i < (size); i++)                            \
                    printf("%d ", (got)[i]);                            \
                putchar('\n');                                          \
                printf("%s:%d: %s: expected:\n",                        \
                       __FILE__, __LINE__, __func__);                   \
                for (i = 0; i < (size); i++)                            \
                    printf("%d ", (expected)[i]);                       \
                putchar('\n');                                          \
                return 1;                                               \
            }                                                           \
        }                                                               \
    } while (0)

static int bad_char_test(const char *pattern, const char *indexes,
                         const uint32_t *values)
{
    uint32_t bad_char[ALPHABET_SIZE];
    uint32_t bad_char_correct[ALPHABET_SIZE];

    const uint32_t pattern_len = (uint32_t)strlen(pattern);

    for (uint32_t i = 0; i < ALPHABET_SIZE; i++) {
        bad_char_correct[i] = pattern_len;
    }

    while (*indexes != '\0') {
        bad_char_correct[*indexes] = *values;
        indexes++;
        values++;
    }

    build_bad_char_array(bad_char, pattern, pattern_len);

    assert_equal_array(bad_char,
                       bad_char_correct,
                       ALPHABET_SIZE);

    return 0;
}

static int good_suffix_test(const char *test_string,
                            const uint32_t correct_good_suffix[])
{
    const size_t string_len = strlen(test_string);
    uint32_t good_suffix[string_len + 1];

    build_good_suffix_array(good_suffix, test_string, string_len);

    assert_equal_array(good_suffix,
                       correct_good_suffix,
                       sizeof(good_suffix)/sizeof(good_suffix[0]));

    return 0;
}

#define assert_equal(test, i)                                   \
    do {                                                        \
        int got = (test);                                       \
        int expected = (int)(i);                                \
        if (got != expected) {                                  \
            printf("%s:%d: %s: expected %d, got %d!\n",         \
                   __FILE__, __LINE__,__func__, expected, got); \
            return 1;                                           \
        }                                                       \
    } while (0)

static int bm_test(void)
{
    const char *pattern;
    const char *text;


    text = "foo";
    pattern = "bar";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 0);

    text = "fo";
    pattern = "foo";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 0);

    text = "foooob";
    pattern = "fooooo";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 0);

    text = "foo";
    pattern = "foo";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 1);

    text = "barfoo";
    pattern = "foo";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 1);

    text = "barfoo rfoobarr foobar";
    pattern = "foobar";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 2);

    text = "foobafoobafoobafoobafrboofarfoobar";
    pattern = "foobar";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 1);

    text = "HOOWOOWOOHOOWOOWOO";
    pattern = "WOOHOO";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 1);

    text = "SALLILAILLATAVANLALLILLALLALALLI";
    pattern = "LALLILLA";
    printf("searching '%s' from '%s'\n", pattern, text);
    assert_equal(boyer_moore(text, pattern), 1);

    return 0;
}

#define run_test(test)                          \
    do {                                        \
        if (test) {                             \
            printf("%s:%d: test failed!\n",     \
                   __FILE__, __LINE__);         \
            return 1;                           \
        }                                       \
    } while (0)

int main(void)
{
    run_test(bad_char_test("WOOHOO", "WOH", (const uint32_t []){5, 0, 2}));
    run_test(bad_char_test("LALLILLA", "AIL", (const uint32_t []){0, 3, 1}));
    run_test(bad_char_test("ABCDB", "ABCD", (const uint32_t []){4, 0, 2, 1}));
    run_test(bad_char_test("ABBABAB", "AB", (const uint32_t []){1, 0, 7}));
    run_test(bad_char_test("GCAGAGAG", "ACG", (const uint32_t []){1, 6, 0}));

    run_test(good_suffix_test("abbabab",
                              (const uint32_t []){5, 5, 5, 5, 2, 5, 4, 1}));
    run_test(good_suffix_test("WOOHOO",
                              (const uint32_t []){6, 6, 6, 6, 3, 1, 1}));
    run_test(good_suffix_test("LALLILLA",
                              (const uint32_t []){6, 6, 6, 6, 6, 6, 6, 8, 1}));
    run_test(good_suffix_test("ABCDABC",
                              (const uint32_t []){4, 4, 4, 4, 4, 7, 7, 1}));
    run_test(good_suffix_test("GCAGAGAG",
                              (const uint32_t []){7, 7, 7, 7, 2, 7, 4, 7, 1}));

    run_test(bm_test());

    return 0;
}
