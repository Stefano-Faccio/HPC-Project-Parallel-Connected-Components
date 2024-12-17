#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage: convert <filename>" << endl;
        return 1;
    }

    ifstream in(argv[1]);
    if (!in)
    {
        cout << "Cannot open input file." << endl;
        return 1;
    }

    ofstream out("output.txt");
    if (!out)
    {
        cout << "Cannot open output file." << endl;
        return 1;
    }
    int nodes = 65608366;
    int edges = 1806067135;
    out << nodes << " " << edges << endl;

    //Se leggo un '#' ignoro la riga
    //Se leggo un numero lo scrivo su un file
    //Se leggo un '\t' scrivo un ' ' su un file

    char c;
    while (in.get(c))
    {
        if (c == '#')
        {
            while (in.get(c) && c != '\n')
                ;
        }
        else if (c == '\t')
        {
            out.put(' ');
        }
        else if (c >= '0' && c <= '9')
        {
            out.put(c);
        }
        else if (c == '\n')
        {
            out.put('\n');
        }
        else
        {
            cout << "Invalid character: " << c << endl;
            return 1;
        }
    }
    
    //Chiudo i file
    in.close();
    out.close();

    return 0;
}
 