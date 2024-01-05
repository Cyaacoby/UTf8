// Carolyn Yaacoby: Computing systems: Professor Ari Shamash & Eli Cohn: Final Project

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//scratch work to see the getbitrange working
// 1010 1110 0010 1101      (3 to 7)
// 0001 0101 1100 0101

// get the bits of num in bit range [start, end)-  bit at location is 0 is the least significant bit
//shifts 'num' right by 'start' bits and 'blocks' out everything but the (end-start) bits
#define GET_BIT_RANGE(num, start, end) (((num) >> (start)) & ((1 << ((end) - (start)))-1))

//just looks at the most significant bits to make sure there are 2 (must start with a 10)
#define IS_CONTINUATION_BYTE(num) (GET_BIT_RANGE(num, 6, 8) == 2)

//checks if character c is a valid hexadecimal val (0-9, A-F, or a-f)
int is_char_hex(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

// Convert hex char into a numerical value in range 0 - 15. Returns -1 if character is not a valid hex char.
// if c is 0-9 it subtracts 0 to get the number, if c is A-F or a-f it subtracts A or a and adds 10
char get_val_from_hex_char(char c) {
    char val = -1;
    if (c >= '0' && c <= '9') {
        val = c - '0';
    } else if (c >= 'A' && c <= 'F') {
        val = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') { // in case hex is lowercase
        val = c - 'a' + 10;
    }
    return val;
}

// Convert val (in range 0-15) into a hex character. Returns -1 if value is above 15.
// if the val is 0-9 add 0, if val is 10-15 add 'A' after adjusting range by -10
char get_hex_char_from_val(int val) {
    char hexChar = -1;
    if (val >= 0 && val <= 9) {
        hexChar = val + '0';
    } else if (val >= 0x0A && val <= 0x0F) {
        hexChar = val + 'A' - 10;
    }
    return hexChar;
}

// Convert code point into UTF8 encoding, returns the num of bytes used in the encoding
int convert_code_point_to_utf8(int codePoint, char *output) {
    if (codePoint <= 0x007F) { // encoding an ASCII character-- just copy ASCII char to output
        *output = codePoint;
        return 1;
    } else if (codePoint <= 0x07FF) { // Need two bytes to encode the codepoint
        output[0] = 0xC0 | GET_BIT_RANGE(codePoint, 6, 11);
        output[1] = 0x80 | GET_BIT_RANGE(codePoint, 0, 6);
        return 2;
    } else if (codePoint <= 0xFFFF) { // Need three bytes to encode the codepoint
        output[0] = 0xE0 | GET_BIT_RANGE(codePoint, 12, 16);
        output[1] = 0x80 | GET_BIT_RANGE(codePoint, 6, 12);
        output[2] = 0x80 | GET_BIT_RANGE(codePoint, 0, 6);
        return 3;
    } else if (codePoint <= 0x10FFFF) { // Need four bytes to encode the codepoint
        output[0] = 0xF0 | GET_BIT_RANGE(codePoint, 18, 21);
        output[1] = 0x80 | GET_BIT_RANGE(codePoint, 12, 18);
        output[2] = 0x80 | GET_BIT_RANGE(codePoint, 6, 12);
        output[3] = 0x80 | GET_BIT_RANGE(codePoint, 0, 6);
        return 4;
    //
    } else {
        assert(0); // used to test function- will crash program if doesn't run correctly (should never reach here!0
    }
}

// Check if string is a valid utf8 encoded string by iteratively checking each byte and its continuation bytes, will return 1 if its valid and 0 if not
int my_utf8_check(char *string) {
    // based off of https://en.wikipedia.org/wiki/UTF-8#Encoding
    int i=0; // index in input encoded string
    while (string[i]) { // Loop through encoded string
        if (GET_BIT_RANGE(string[i], 7, 8) == 0) {
            i += 1;
        } else if (GET_BIT_RANGE(string[i], 5, 8) == 6) { // if this is true it's a 2 byte encoding (110)
            int validContinuationBytes = IS_CONTINUATION_BYTE(string[i+1]); //checks that the next byte is a continuation byte
            if (!validContinuationBytes) return 0; //if it's not a continuation byte then the string isn't valid
            i += 2;
        } else if (GET_BIT_RANGE(string[i], 4, 8) == 14) { //if its true it's a 3 byte encoding
            // check if string[i+1] is zero so that we can break the loop immediately. If we didn't check this, program may have an error when checking string[i+2]
            if (string[i+1] == 0) return 0; //edge case just checking to make sure theres more bytes
            int validContinuationBytes = IS_CONTINUATION_BYTE(string[i+1]) && IS_CONTINUATION_BYTE(string[i+2]);
            if (!validContinuationBytes) return 0;
            i += 3;
        } else if (GET_BIT_RANGE(string[i], 3, 8) == 30) {
            // check if string[i+1] or string[i+2] is zero so that we can break the loop immediately. If we didn't chcek this, program may have an error when checking string[i+2] or string[i+3]
            if (string[i+1] == 0 || string[i+2] == 0) return 0;
            int validContinuationBytes = IS_CONTINUATION_BYTE(string[i+1]) && IS_CONTINUATION_BYTE(string[i+2]) && IS_CONTINUATION_BYTE(string[i+3]);
            if (!validContinuationBytes) return 0;
            i += 4;
        } else {
            return 0; // not a valid UTF8 encoding if the above if statements are not satisfied
        }
    }
    // if the above while loop went through the whole string without returning 0, then the UTF8 string is valid.
    return 1;
}

// gets the length o the UTF-8 encoded string in terms of # of chars (not bytes)
int my_utf8_strlen(char *string) {
    // if not a valid string, return -1
    if (my_utf8_check(string) == 0) {
        return -1;
    }
    // Based off of https://en.wikipedia.org/wiki/UTF-8#Encoding
    int i=0; // index in input encoded string
    int myStrLen = 0;
    while (string[i]) { // Loop through string encoded string
        if (GET_BIT_RANGE(string[i], 7, 8) == 0) { //if its 1 byte
            i += 1;
            myStrLen++;
        } else if (GET_BIT_RANGE(string[i], 5, 8) == 6) { //if its 2 bytes
            i += 2;
            myStrLen++;
        } else if (GET_BIT_RANGE(string[i], 4, 8) == 14) { //if its 3 bytes
            i += 3;
            myStrLen++;
        } else if (GET_BIT_RANGE(string[i], 3, 8) == 30) { //if its 4 bytes
            i += 4;
            myStrLen++;
        } else {
            assert(0); // should never happen! (the else)
        }
    }
    return myStrLen;
}

// Return the UTF8 encoded character at position index in the string
// here, the index refers to the character position in string not byte position
char *my_utf8_charat(char *string, int index) {
    // see if string is valid UTF-8 encoded string, if not will return NULL
    if (my_utf8_check(string) == 0) {
        return NULL;
    }
    // based off of https://en.wikipedia.org/wiki/UTF-8#Encoding
    int i=0; // i tracks the current byte position in string
    int charIdx = 0; //charIdx tracks current character position
    while (string[i]) { // Loop through string encoded string
        if (GET_BIT_RANGE(string[i], 7, 8) == 0) { //if current char is 1 byte
            if (charIdx == index) {

                char *substr = malloc(1 + 1); // allocate 1 extra byte for the null terminator
                strncpy(substr, &string[i], 1); // copy the encoded char
                substr[1] = '\0';
                return substr;
            }
            i += 1;
            charIdx++;
        } else if (GET_BIT_RANGE(string[i], 5, 8) == 6) { //if current char is 2 byte
            if (charIdx == index) {
                char *substr = malloc(2 + 1); // allocate 2 bytes and 1 extra for the null terminator
                strncpy(substr, &string[i], 2); // copy the encoded char into this memory
                substr[2] = '\0'; //adds null terminator 0 (to indicate string is done) to last char in string
                return substr;
            }
            i += 2;
            charIdx++;
        } else if (GET_BIT_RANGE(string[i], 4, 8) == 14) { //if current char is 3 byte
            if (charIdx == index) {
                char *substr = malloc(3 + 1); // allocate 1 extra byte for the null terminator
                strncpy(substr, &string[i], 3); // copy the encoded char
                substr[3] = '\0';
                return substr;
            }
            i += 3;
            charIdx++;
        } else if (GET_BIT_RANGE(string[i], 3, 8) == 30) { //if current char is 4 byte
            if (charIdx == index) {
                char *substr = malloc(4 + 1); // allocate 1 extra byte for the null terminator
                strncpy(substr, &string[i], 4); // copy the encoded char
                substr[4] = '\0';
                return substr;
            }
            i += 4;
            charIdx++;
        } else {
            assert(0); // should not be in this else statement
        }
    }
    return NULL;
}

// input is passed in as ASCII string with UTF chars passed in as \u...
// output is a UTF-8 encoded string
int my_utf8_encode(char *input, char *output) {
    int i=0;
    char *outputStart = output; //keeps track of the beginning of the output
    while (input[i]) { // Loop through input (non-decoded) string
        int codePoint = 0;
        if (input[i] == '\\' && input[i + 1] == 'u') { //if current and next chars are \&u then its unicode and we move forward
            i += 2; // advance past the '\u' (two positions)


            // the code point can be a maximum of 6 hex digits (U+10FFFF) and we need one more for the null terminator
            char hexStr[7] = {0}; //array that can fit 7 chars all of which are initialized to 0
            char *hexPtr = hexStr; //hexPtr is a pointer that will start at the beginning of hexStr and will fill it with hex chars
            // get all hex characters in input string (meaning the characters (xxxx) in \uxxxx)
            while (is_char_hex(input[i])) {
                *hexPtr++ = input[i++]; //ir char is hex add it to hexStr through hexPtr
            }
            int hexStrLen = hexPtr - hexStr; // get string len
            // converts he string into its unicode point
            for (int j = 0; j < hexStrLen; j++) {
                //the value of each hex digit depends on its position (the rightmost is 16^0 which is 1) and each digit to the left is 16x larger than its right digit
                //so to convert, we multiply each digits value by its position weight and then sum
                codePoint += get_val_from_hex_char(hexStr[j]) * pow(16, hexStrLen - j - 1);
            }
            // C * 16^0 + A * 16^1 + 0 * 16^2 ...
            // code point will be converted to UTF-8 bytes, then Output is filled in
            int encodingLen = convert_code_point_to_utf8(codePoint, output);
            output += encodingLen; // move on to next part of output string
        }
        else {
            // if the char is a regular ascii char, will go into this else statement and pass it to the output directly
            *output++ = input[i];
            i++;
        }
    }
    int encodedStringLen = output - outputStart; // get encoded string len after processing entire string by subtracking original position of output from its curr position
    output[encodedStringLen] = '\0'; // append \0 at the end for null terminate
    return encodedStringLen;
}

// input is a UTF8 encoded string and returns a decoded string. If UTF8 char is representable in ASCII, then put ASCII in the decoded string. If UTF8 characters are not representable in ASCII, put their code point representation in the decoded string.
int my_utf8_decode(char *input, char *output) {
    // Based off of https://en.wikipedia.org/wiki/UTF-8#Encoding
    int i=0; // index in input encoded string
    char *outputStart = output;
    while (input[i]) { // Loop through input encoded string
        if (GET_BIT_RANGE(input[i], 7, 8) == 0) { //for 1 byte
            *output++ = input[i];
            i += 1;
        } else if (GET_BIT_RANGE(input[i], 5, 8) == 6) { //for 2 bytes
            *output++ = '\\';
            *output++ = 'u';
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i], 2,5));
            //extracts the LSB (0,1), shifts them left 2 positions
            // extract bits from next byte sequence
            // perform bitwise OR between shifted bits input[i] and bits from input[i+1] which will combine these sets of bits into a single value
            *output++ = get_hex_char_from_val((GET_BIT_RANGE(input[i], 0,2) << 2) | GET_BIT_RANGE(input[i+1], 4, 6));
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i+1], 0,4));
            i += 2;
        } else if (GET_BIT_RANGE(input[i], 4, 8) == 14) { //for 3 bytes
            *output++ = '\\';
            *output++ = 'u';
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i], 0,4));
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i+1], 2,6));
            *output++ = get_hex_char_from_val((GET_BIT_RANGE(input[i+1], 0,2) << 2) | GET_BIT_RANGE(input[i+2], 4, 6));
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i+2], 0,4));
            i += 3;
        } else if (GET_BIT_RANGE(input[i], 3, 8) == 30) { // for 4 bytes
            *output++ = '\\';
            *output++ = 'u';
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i], 2,3));
            *output++ = get_hex_char_from_val((GET_BIT_RANGE(input[i], 0,2) << 2) | GET_BIT_RANGE(input[i+1], 4, 6));
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i+1], 0,4));
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i+2], 2,6));
            *output++ = get_hex_char_from_val((GET_BIT_RANGE(input[i+2], 0,2) << 2) | GET_BIT_RANGE(input[i+3], 4, 6));
            *output++ = get_hex_char_from_val(GET_BIT_RANGE(input[i+3], 0,4));
            i += 4;
        } else {
            assert(0); // should not be in this else statement
            return -1;
        }
    }
    int decodedStrLen = output - outputStart;
    return decodedStrLen;
}

// Additional function
//compares 2 strings and count bitwise difference
int my_utf8_strcmp(char *string1, char *string2) {
    // Based off of https://en.wikipedia.org/wiki/UTF-8#Encoding
    int i=0; // index in input encoded string
    // Loop through strings until you find a bitwise difference. If not, return 0
    while (string1[i] - string2[i] == 0) {
        if (string1[i] == 0 && string2[i] == 0) return 0;
        i += 1;
    }
    return (unsigned char) string1[i] - (unsigned char) string2[i];
}

// Additional function
// Return if UTF8 encoded strings are equal
int my_utf8_eq(char *string1, char *string2) {
    return my_utf8_strcmp(string1, string2) == 0;
}

//additional function to show how much memory you are saving through UTF8 encoding compared to regular storing
// Given a UTF8 encoded string, compute the memory that is saved due to the UTF8 encoding compared to storing each character's code point, which requires 4 bytes per character. Returns the ratio of memory saved over memory that would be used to store a string using code points directly.
double my_utf8_memory_savings(char *string) {
    // Based off of https://en.wikipedia.org/wiki/UTF-8#Encoding
    int i=0; // index in input encoded string
    while (string[i]) { // Loop through string encoded string
        if (GET_BIT_RANGE(string[i], 7, 8) == 0) {
            i += 1;
        } else if (GET_BIT_RANGE(string[i], 5, 8) == 6) {
            i += 2;
        } else if (GET_BIT_RANGE(string[i], 4, 8) == 14) {
            i += 3;
        } else if (GET_BIT_RANGE(string[i], 3, 8) == 30) {
            i += 4;
        } else {
            assert(0); // should not be in this else statement
        }
    }

    int numBytesInUTF8Encoding = i + 1; // + 1 for the null terminator
    int numBytesInCodePointRepresentation = 4 * my_utf8_strlen(string) + 1; // Need 4 bytes per character in code point representation (if you're assuming it will take the most bytes), + 1 for the null terminator

    int numBytesSaved = numBytesInCodePointRepresentation - numBytesInUTF8Encoding;

    return (double) numBytesSaved / numBytesInCodePointRepresentation;
}

int main() {
    // \u, where anything following is the code point of the character. Each character has a unique code point.
    //TEST EXAMPLES
    //char *test_string = "\\u004D\\u00EC\\u006E\\u0068\\u0020\\u006E\\u00F3\\u0069"; // Corresponds to 'Mình nói'
    //char *test_string = "\\u05D0\\u05E8\\u05D9\\u05D4"; // Corresponds to 'אריה'
    char *test_string = "Hello \\u05D0\\u05E8\\u05D9\\u05D4"; // Corresponds to 'Hello אריה'



    printf("Test string: %s\n", test_string);
    //allocates memomry for the encoded string
    //passing into malloc the # of bytes i want to allocate
    char *encodedString = malloc(strlen(test_string));
    // encoded string will hold the UTF-8 encoded string obtained from the code points in the test.
    //encodes the raw string into the UTF-8 encoding and returns the length of that string
    int lenEncodedString = my_utf8_encode(test_string, encodedString);
    printf("Encoded string: %s\n", encodedString); // encoded string is UTF8 encoded, so it will print correctly using printf.

    //checking if the encoded string was validly converted to UTF-8
    int is_valid_utf8_string = my_utf8_check(encodedString);
    printf("Is valid UTF8 string? %s\n", is_valid_utf8_string ? "true" : "false");

    //test to see the string length --> will output length #
    int myStrLen = my_utf8_strlen(encodedString);
    printf("my_utf8_strlen of encoded string: %d\n", myStrLen);

    //test to see if the characters correctly print at given pos --> will give the letter at the given position
    char *charAt1 = my_utf8_charat(encodedString,1);
    printf("Char at pos 1: %s\n", charAt1);
    free(charAt1); // due to malloc, the user (me) has to 'release' the memory/data

    char *charAt2 = my_utf8_charat(encodedString, 2);
    printf("Char at pos 2: %s\n", charAt2);
    free(charAt2);

    char *charAt6 = my_utf8_charat(encodedString, 6);
    printf("Char at pos 6: %s\n", charAt6);
    free(charAt2);

    //test the encoding savings function to show how much you are saving
    double encodingSavings = my_utf8_memory_savings(encodedString);
    printf("Encoding savings of string '%s' due to UTF8: %lf\n", encodedString, encodingSavings);

    // potential additional function: optimize how large encodedString is, right now it is too conservative
    char *decodedString = malloc(2 * strlen(encodedString)); // strlen will return # of bytes when passed in a UTF8 encoded string
    int lenDecodedString = my_utf8_decode(encodedString, decodedString);
    printf("Decoded string: %s\n", decodedString);

    free(encodedString);
    free(decodedString);

    printf("\n\n");

    // test the string comparison function
    // ascii value of e (in bits):   0110 0101    (in decimal: 101)
    // utf8 encoding of é (in bits): 1100 0011 1010 1001    (in decimal, 195 169)
    // while they look like the same because of the ' on top of the e in the second 'cafe' they will have different values
    //char *string1 = "Cafe";
    //char *string2 = "Café";

    //additional test
    char *string1 = "Hello";
    char *string2 = "Héllo";

    //compare the string
    int myStrCmp = my_utf8_strcmp(string1, string2);
    printf("my_utf8_strcmp between %s and %s: %d\n", string1, string2, myStrCmp);

    //check the string equals function
    int myStrEq = my_utf8_eq(string1, string2);
    printf("my_utf8_eq between %s and %s: %s\n", string1, string2, myStrEq ? "true" : "false");

}
