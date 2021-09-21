#include <iostream>
#include <optional>

#include "editor.h"

std::optional<std::string> retrieve_filename(int charc, char** charv)
{
    if (charc == 2)
    {
        return charv[1];
    }
    else
    {
        std::cout << "\nusage: pilo [filename]\n";
        return std::nullopt;
    }
}

// This comment was written using this editor :)

int main(int charc, char** charv)
{
    auto filename = retrieve_filename(charc, charv);
    if (!filename.has_value())
        return EXIT_FAILURE;

    Pilo::Editor app;
    app.run(filename.value());

    return EXIT_SUCCESS;
}
