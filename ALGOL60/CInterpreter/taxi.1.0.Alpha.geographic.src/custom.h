/* custom.h 
 * Copyleft (C) 2009-2014 Antonio Maschio 
 * @ <ing dot antonio dot maschio at gmail dot com>
 * Update: 15 novembre 2020
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This files contains customizable entities
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

/* CUSTOMIZABLE ENTITIES */

/* this is the default height of a character printed page. To set your own
 * height, follow the instructions on the manual  */
#define DEFPAGEHEIGHT 68

/* printing shell command. This should be enough for any Linux/UNIX OS
 * but if you don't have lpr or if you want to use another printing tool,
 * set it here. Add the path if necessary */
#define ROUTESTRING "lpr"

/* set the maximum number of loadable libraries by EXTERNAL. This seems 
 * to me a sufficient value. Change at will. */
#define EXTERNLIBMAX  64

/* the executable to be used for CALL; if you've managed to install taxi
 * you can set it to "taxi"; if you want to keep it on the install
 * directory (or wherever you've copied it), set it to "./taxi " 
 * Watch out! The final space is needed */
#define CALLEX "./taxi "
