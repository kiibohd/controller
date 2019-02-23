/* Copyright (C) 2018-2019 by Jacob Alexander and Rowan Decker
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

#include <Macro/PartialMap/kll.h>
#include <Lib/storage.h>
#include <print.h>
#include <cli.h>



// ----- CLI -----

void cliFunc_storage( char* args );
void cliFunc_load( char* args );
void cliFunc_save( char* args );
void cliFunc_defaults( char* args );

CLIDict_Entry( storage,  "Print settings storage info" );
CLIDict_Entry( load, "Load saved settings" );
CLIDict_Entry( save, "Save current settings" );
CLIDict_Entry( defaults, "Load default settings" );

CLIDict_Def( storageCLIDict, "Storage Module Commands" ) = {
	CLIDict_Item( storage ),
	CLIDict_Item( load ),
	CLIDict_Item( save ),
	CLIDict_Item( defaults ),
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Variables -----

StorageModule* storage_modules[StorageMaxModules];
uint8_t module_count;



// ----- Functions -----

void Storage_init() {
	storage_init();
	storage_default_settings();
	CLI_registerDictionary( storageCLIDict, storageCLIDictName );
}

void Storage_registerModule(StorageModule *config) {
	storage_modules[module_count++] = config;
}

uint8_t storage_load_settings() {
	uint8_t buffer[STORAGE_SIZE];

	uint8_t success = storage_read(buffer, 0, sizeof(buffer)) == 1;
	if (success) {
		uint8_t offset = 0;
		for (uint8_t i=0; i<module_count; i++) {
			if (storage_is_storage_cleared()) {
				memcpy(storage_modules[i]->settings, storage_modules[i]->defaults, storage_modules[i]->size);
			} else {
				memcpy(storage_modules[i]->settings, buffer+offset, storage_modules[i]->size);
			}
			storage_modules[i]->onLoad();
			offset += storage_modules[i]->size;
		}
	}
	return success;
}

uint8_t storage_save_settings() {
	uint8_t buffer[STORAGE_SIZE];
	uint8_t offset = 0;
	for (uint8_t i=0; i<module_count; i++) {
		storage_modules[i]->onSave();
		memcpy(buffer+offset, storage_modules[i]->settings, storage_modules[i]->size);
		offset += storage_modules[i]->size;
	}
	return storage_write(buffer, 0, sizeof(buffer), 0) == 1;
}

void storage_default_settings() {
	for (uint8_t i=0; i<module_count; i++) {
		memcpy(storage_modules[i]->settings, storage_modules[i]->defaults, storage_modules[i]->size);
	}
}

void cliFunc_storage( char* args )
{
	print( NL );
	print("Page: ");
	printHex( storage_page_position() );
	print(", Block: ");
	printHex( storage_block_position() );
	print( NL );
	print("Address: ");
	printHex32((STORAGE_FLASH_START
		+ storage_page_position() * STORAGE_FLASH_PAGE_SIZE)
		+ (storage_block_position() * (STORAGE_SIZE + 1))
	);
	print( NL );
	print("Cleared?: ");
	printInt8( storage_is_storage_cleared() );
	print( NL );

	for (uint8_t i=0; i<module_count; i++) {
		print( NL "\033[1;32m" );
		print(storage_modules[i]->name);
		print(" Storage\033[0m" NL );
		storage_modules[i]->display();
	}
}

void cliFunc_load( char* args ) {
	print( NL );
	if (storage_load_settings()) {
		print("Done!");
	} else {
		print("Error!");
	}
}

void cliFunc_save( char* args )
{
	print( NL );
	if (storage_save_settings()) {
		print("Success!");
	} else {
		print("Failure!");
	}
}

void cliFunc_defaults( char* args )
{
	/*print( NL );
	switch ( storage_clear_page() )
	{
	case 0:
		print("Already cleared" NL);
		break;
	default:
		print("Flash cleared!" NL);
		break;
	}*/

	storage_default_settings();
	for (uint8_t i=0; i<module_count; i++) {
		storage_modules[i]->onLoad();
	}

	print( NL "Loaded defauts");
}

void Storage_StorageControl_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Storage_StorageControl_capability(func)");
		return;
	default:
		return;
	}

	uint8_t arg  = *(uint8_t*)(&args[0]);

	// Decide how to handle function
	switch ( arg )
	{
	case 0: // Load
		storage_load_settings();
		break;
	case 1: // Save
		storage_save_settings();
		break;
	case 2: // Defaults
		storage_default_settings();
		for (uint8_t i=0; i<module_count; i++) {
			storage_modules[i]->onLoad();
		}
		break;
	}
}

