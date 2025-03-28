/*
  keyboard map for when shift is pressed
*/
unsigned char keyboard_map_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',   // 9
  '(', ')', '_', '+', '\b',  // Backspace
  '\t',                      // Tab
  'Q', 'W', 'E', 'R',        // 19
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',  // Enter
    0,                       // Control
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', // 39
 '"', '~',  0,   // Left Shift
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',        // 49
  'M', '<', '>', '?',   0,      // Right Shift
  '*', 0, ' ', 0, // Space, Caps Lock
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Function keys (F1-F10)
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // More function keys
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // More special keys
};
