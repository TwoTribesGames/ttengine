#if !defined(INC_TT_SYSTEM_UTILS_H)
#define INC_TT_SYSTEM_UTILS_H


#include <string>

#include <tt/str/str_types.h>


namespace tt {
namespace system {


/*! \brief Open a file or path with the OS default application
    \param p_item The item to open 
    \return True if the operation has succeeded */
bool openWithDefaultApplication(const std::string& p_item);

/*! \brief Edit a file or path with the OS default application
    \param p_item The item to edit
    \return True if the operation has succeeded */
bool editWithDefaultApplication(const std::string& p_item);

/*! \brief Shows the specified file in the OS default file navigator (Explorer, Finder, etc).
    \param p_item The item to show.
    \return True if the operation has succeeded */
bool showFileInFileNavigator(const std::string& p_item);


/*! \brief Puts the lines of text in p_lines on the system clipboard.
    \param p_lines The text to put on the system clipboard.
    \return True if the operation has succeeded. */
bool setSystemClipboardText(const str::Strings& p_lines);

/*! \brief Retrieves the text on the system clipboard and splits it into separate lines.
    \param p_lines_OUT The container that should receive the lines of text.
    \return True if the operation has succeeded. This function can fail if the system clipboard
            does not contain anything, or if the clipboard does not contain text. */
bool getSystemClipboardText(str::Strings* p_lines_OUT);

/*! \brief Retrieves the file path to the system desktop
    \return A string containing the path to the desktop */
std::string getDesktopPath();

// Namespace end
}
}


#endif  // !defined(INC_TT_SYSTEM_UTILS_H)
