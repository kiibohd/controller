/* Copyright (C) 2016 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// ----- Macros -----

// Convenience Macros
#define gpio( port, pin ) { Port_##port, Pin_##pin }
#define sense( port, pin, adc, ch ) { Port_##port, Pin_##pin, ADC_##adc, Channel_##ch }
#define Matrix_colsNum sizeof( Matrix_cols ) / sizeof( GPIO_Pin )
#define Matrix_rowsNum sizeof( Matrix_rows ) / sizeof( GPIO_Pin )
#define Matrix_maxKeys sizeof( Matrix_scanArray ) / sizeof( KeyState )

