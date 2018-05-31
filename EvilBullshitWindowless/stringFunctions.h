// ugly function to trasnform virtual key codes into ascii
char VKtoASCII(int vkcode, bool shift) {
	switch (vkcode) {
	case 56:
		vkcode = shift ? '*' : '8';
		break;
	case 186:
		vkcode = shift ? ':' : ';';
		break;
	case 187:
		vkcode = shift ? '+' : '=';
		break;
	case 188:
		vkcode = shift ? '<' : ',';
		break;
	case 189:
		vkcode = shift ? '_' : '-';
		break;
	case 190:
		vkcode = shift ? '>' : '.';
		break;
	case 191:
		vkcode = shift ? '?' : '/';
		break;
	case 192:
		vkcode = shift ? '~' : '`';
		break;
	case 219:
		vkcode = shift ? '{' : '[';
		break;
	case 220:
		vkcode = shift ? '|' : '\\';
		break;
	case 221:
		vkcode = shift ? '}' : ']';
		break;
	case 222:
		vkcode = shift ? '"' : '\'';
		break;
	}

	if (vkcode >= 106 && vkcode <= 111) {
		vkcode -= 64;
	}
	if (!shift) {
		// fix symbols
		if (vkcode >= 96 && vkcode <= 105) {
			vkcode -= 48;
		}
		else {
			// lower case letters
			if (vkcode >= 65 && vkcode <= 90) {
				vkcode += 32;
			}
		}
	}
	else {
		if (vkcode >= 48 && vkcode <= 57) {
			vkcode -= 16;
		}
	}
	if (vkcode < 129 && vkcode > 1) {
		return vkcode;
	}
	else {
		return 0;
	}
}

// string helper func
bool endsWith(string const & value, string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// returns number of characters to backspace if a key phrase is triggered
int typedKeyPhrase(vector<char> &keys) {
	auto str = std::string(keys.begin(), keys.end());
	for (auto&& key : KeyWords) {
		if (endsWith(str, key))
			return key.length();
	}
	return 0;
}

// trigger windows to type a letter as though the keyboard had done it
void triggerKey(char key) {
	// Set up a generic keyboard event.
	auto lower = key > 96 && key < 123;
	auto capital = key > 64 && key < 91;
	auto inputSize = sizeof(INPUT);
	INPUT ip, ip2;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.wVk = lower ? key - 32 : key; // virtual-key code for the key
	ip.ki.dwFlags = 0; // 0 for key press

					   // send shift key signal if letter is capital
	if (capital) {
		ip2.type = INPUT_KEYBOARD;
		ip2.ki.wScan = 0;
		ip2.ki.time = 0;
		ip2.ki.dwExtraInfo = 0;
		ip2.ki.wVk = VK_SHIFT; // virtual-key code for the "shift" key
		ip2.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip2, inputSize);
	}

	// send key signal followed by keyup signal
	SendInput(1, &ip, inputSize);
	Sleep(1);
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, inputSize);

	// unpress shift key if needed
	if (capital) {
		ip2.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip2, inputSize);
	}
}

// make windows type out a string as though the keyboard had done it
void typeString(std::string text) {
	for (size_t i = 0; i < text.size(); i++) {
		triggerKey(text.at(i));
	}
}

// returns lowercase representation of character
char lowercase(char ch) {
	return ch > 64 ? ch < 91 ? ch + 32 : ch : ch;
}