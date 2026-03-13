/*

______  ___  _____       _____              _         _ 
|  ___|/   ||_   _|     /  ___|            (_)       | |
| |_  / /| |  | |  ___  \ `--.   ___  _ __  _   __ _ | |
|  _|/ /_| |  | | / _ \  `--. \ / _ \| '__|| | / _` || |
| |  \___  |  | || (_) |/\__/ /|  __/| |   | || (_| || |
\_|      |_/  \_/ \___/ \____/  \___||_|   |_| \__,_||_|
                                                        
    Copyright (C) F4ToSerial By Myoda, Inc - All Rights Reserved
    Unauthorized copying of this file, via any medium is strictly prohibitedsss
    Proprietary and confidential  
    For more informations, please see : https://f4toserial.com/

*/
/* ------- COMMONS ------- */
int readline(int readch, char *buffer, int len) {
    static int pos = 0;
    int rpos;

    if (readch > 0) {
        switch (readch) {
            case '\n': // Ignore new-lines
                break;
            case '\r': // Return on CR
                rpos = pos;
                pos = 0;  // Reset position index ready for next time
                return rpos;
            default:
                if (pos < len-1) {
                    buffer[pos++] = readch;
                    buffer[pos] = 0;
                }
        }
		return 0;		// added by kimhy
    }
    // No end of line has been found, so return -1.
    return -1;
}

// Reset readline buffer (for protocol switching)
void resetReadline() {
    // Force pos back to 0 by calling readline with a fake CR
    // This ensures the static pos inside readline() is reset.
    char dummy[4];
    readline('\r', dummy, sizeof(dummy));
}
