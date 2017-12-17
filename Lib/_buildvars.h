/* Copyright (C) 2013-2017 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

// ----- Includes -----



// ----- Defines -----

// Helpers
#define _STR_HELPER(x)
#define _STR(x) _STR_HELPER(x)

// You can change these to give your code its own name.
#define STR_MANUFACTURER        L"@MANUFACTURER@"
#define STR_PRODUCT             L"Keyboard - @ScanModule@@LayoutName@ @MacroModule@ @OutputModule@"
#define STR_SERIAL              L"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX - @CHIP_SHORT@"
#define STR_CONFIG_NAME         L"@FLASHING_STATION_ID@"


// Strings used in the CLI module
#define CLI_Revision            "@Git_Commit_Revision@"
#define CLI_RevisionNumber      @Git_Commit_Number@
#define CLI_RevisionNumberStr   "@Git_Commit_Number@"
#define CLI_Version             "@Git_Recent_Tag@"
#define CLI_VersionRevision     "@Git_Recent_Tag_Revision@"
#define CLI_VersionRevNumber    @Git_Recent_Tag_Commit_Number@
#define CLI_VersionRevNumberStr "@Git_Recent_Tag_Commit_Number@"
#define CLI_Branch              "@Git_Branch_INFO@"
#define CLI_ModifiedStatus      "@Git_Modified_Status@"
#define CLI_ModifiedFiles       "@Git_Modified_Files@"
#define CLI_RepoOrigin          "@Git_Origin_URL@"
#define CLI_CommitDate          "@Git_Date_INFO@"
#define CLI_CommitAuthor        @Git_Commit_Author@
#define CLI_Modules             "Scan(@ScanModule@@LayoutName@) Macro(@MacroModule@) Output(@OutputModule@) Debug(@DebugModule@)"
#define CLI_BuildDate           "@Build_Date@"
#define CLI_BuildOS             "@Build_OS@"
#define CLI_BuildCompiler       "@CMAKE_C_COMPILER@ @CMAKE_C_COMPILER_VERSION@"
#define CLI_Arch                "@COMPILER_FAMILY@"
#define CLI_Chip                "@CHIP@"
#define CLI_ChipShort           "@CHIP_SHORT@"
#define CLI_CPU                 "@CPU@"
#define CLI_Device              "Keyboard"


// Mac OS-X and Linux automatically load the correct drivers.  On
// Windows, even though the driver is supplied by Microsoft, an
// INF file is needed to load the driver.  These numbers need to
// match the INF file.
#define VENDOR_ID               @VENDOR_ID@
#define PRODUCT_ID              @PRODUCT_ID@
#define BCD_VERSION             @Git_Commit_Number@

