/* errors.h 
 * Copyleft (C) 2009-2014 Antonio Maschio 
 * @ <ing dot antonio dot maschio at gmail dot com>
 * Update: 04 gennaio 2021
 * 
 * This is taxi, an ALGOL interpreter written for the UNIX world.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file contains the errors and warnings messages issued by taxi.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * It's GPL! Enjoy!
 */

char* error[]= {

/* ALGOL ERRORS */
/*  0 */ "Enabled error condition",
/*  1 */ "File name must be given",
/*  2 */ "Syntax error",
/*  3 */ "Missing DO in WHILE or FOR construct",
/*  4 */ "Warning: missing final dot terminator after last END", /* out */
/*  5 */ "Error -- suspect missing ;",
/*  6 */ "Memory error",
/*  7 */ "Out of memory",
/*  8 */ " is an unrecognized option. Ignored.", /* out */
/*  9 */ "Illegal number",
/* 10 */ "Illegal string, undeclared string variable or undefined string function",
/* 11 */ "Illegal label",
/* 12 */ "Limit of recursion step reached; stopping execution",
/* 13 */ "Illegal subscript",
/* 14 */ "File not open",
/* 15 */ "File is a directory",
/* 16 */ "Error in FOR statement",
/* 17 */ "Error in PROCEDURE call",
/* 18 */ "Unfinished program. Check blocks",
/* 19 */ "Illegal character in string format sequence",
/* 20 */ "Error in IF procedure",
/* 21 */ "ELSE cannot be used as an independent procedure",
/* 22 */ "IF-FOR-WHILE not permitted after THEN, nor FOR-WHILE after ELSE",
/* 23 */ "Irregular program flow. Check blocks",
/* 24 */ "No colon in bound pair",
/* 25 */ "Parameters types do not match",
/* 26 */ "Identifier too long",
/* 27 */ "Subscript out of bounds",
/* 28 */ "Math domain error",
/* 29 */ "Upper bound less than lower bound in array declaration",
/* 30 */ "String too long",
/* 31 */ "Illegal formula",
/* 32 */ "",
/* 33 */ "",
/* 34 */ "Input or output on undefined channel",
/* 35 */ "",
/* 36 */ "Attempt to go past the maximum array dimensions number",
/* 37 */ "",
/* 38 */ "Attempt to read or write over end-of-file",
/* 39 */ "",
/* 40 */ "er in numeric data",
/* 41 */ "",
/* 42 */ "Error condition on closing file",
/* 43 */ "",
/* 44 */ "I/O channel number out of range",
/* 45 */ "Square brackets [ ] not paired in string",
/* 46 */ "Illegal name",
/* 47 */ "Division by zero",
/* 48 */ "SQRT argument negative",
/* 49 */ "LN argument zero or negative",
/* 50 */ "EXP/LEXP argument too large",
/* 51 */ "Inverse maths function argument out of range",
/* 52 */ "typeless PROCEDURE used in typed position",
/* 53 */ "Illegal EXTERNAL name",
/* 54 */ "Spurious double-quote",
/* 55 */ "Illegal assignment",
/* 56 */ "Illegal string variable",
/* 57 */ "Illegal string byte-subscript",
/* 58 */ "Illegal attempt to read from an OUTPUT file",
/* 59 */ "Illegal attempt to write to an INPUT file",
/* 60 */ "Illegal numeric variable",
/* 61 */ "Illegal variable or variable not instantiated",
/* 62 */ "Illegal variable or variable out of scope for DUMP",
/* 63 */ "Illegal string position",
/* 64 */ "File could not be opened or channel could not be selected",
/* 65 */ "Channel conflict",
/* 66 */ "Attempt to use not-transferable channels",
/* 67 */ "File could not be created",
/* 68 */ "Connection not established for OPEN",
/* 69 */ "Duplicated label",
/* 70 */ "Illegal array",
/* 71 */ "Illegal call by value",
/* 72 */ "Missing reference in procedure declaration",
/* 73 */ "Illegal assignment or undeclared untyped procedure call",
/* 74 */ "",
/* 75 */ "Missing procedure body",
/* 76 */ "Attempt to input from channel 1",
/* 77 */ "Attempt to RELEASE a channel with a linked file ",
/* 78 */ "File already open", /* out */
/* 79 */ "Warning: temporary file created for print couldn't be removed.",
/* 80 */ "ARRAY not subscripted",
/* 81 */ "Illegal number, undeclared numeric variable or undefined numeric function",
/* 82 */ "Attempt to output on channel 0",
/* 83 */ "Missing matching argument",
/* 84 */ "Circular EXTERNAL loading is forbidden",
/* 85 */ "",
/* 86 */ "",
/* 87 */ "Illegal numeric array",
/* 88 */ "Illegal assignment due to invalid lefthand type",
/* 89 */ "Integration variable must exist",
/* 90 */ "Illegal FOR procedure",
/* 91 */ "",
/* 92 */ "",
/* 93 */ "",
/* 94 */ "",
/* 95 */ "",
/* 96 */ "",
/* 97 */ "",
/* 98 */ "\nWarning: There's another procedure with the same name %s",
/* 99 */ "\nWarning: Identifier %s at line %d already declared in line %d.\n\n",
/*L+1 */ ""					/* last + 1 */
};

#define ERRLIM 100				/* L + 1 */

