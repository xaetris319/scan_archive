#pragma once

namespace message_type {
	const int NORMAL = 0;
	const int ERROR = 1;
	const int WARNING = 2;
}

namespace message {
	const std::string EMPTY("");
	const std::string NO_INPUT("Error: couldn\'t read input.txt. Check ABAP program.");
	const std::string WRONG_INPUT("Error: input.txt has wrong content. Check ABAP program.");
	const std::string NO_FILES("Error: couldn\'t find any files after NAPS2 performed the scanning.");
}
