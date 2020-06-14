#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <limits.h>
#include <tgmath.h>
#include <climits> 

using namespace std;

//Marco de pagina
struct ProcesoReal {
    int idProceso;
    double timestamp;
    int cantBytes;
};

//Swaping [Virtual]
struct ProcesoVirtual {
    int idProceso;
    int pagina;
    int marcoDePagina;
    double timestamp;
    int cantBytes;
};

//Proceso
struct Proceso {
    int idProceso;
    double tiempoInicio;
    double tiempoFinal;
    int cantPaginas;
    int cantPageFaults;
    int tamProceso;
};

// Vectores de memoria;
vector<ProcesoReal*> M;
vector<ProcesoVirtual*> S;

//Vector de procesos
vector<Proceso> procesos;

// Variables globales
double tiempo;
int tamPagina;
int cantSwaps;
int cantPaginasVirtLibres;
string politica;

/*Funcion donde se inicializa como NULL los vectores (ya que son de apuntadores) */
void valoresIniciales() {
    for (int i = 0; i < 128; i++) {
        M.push_back(NULL);
        S.push_back(NULL);
        S.push_back(NULL);
    }

    cantPaginasVirtLibres = S.size();
    tiempo = 0;
    tamPagina = 16;
    cantSwaps = 0;
}

/*Función utilizada para reiniciar los valores en la función de "fin" */
void reiniciarValores() {
    M.clear();
    S.clear();
    procesos.clear();

    valoresIniciales();
}

string mostrarRangos(vector<int> &vec) {
    string extra = "";
    for (int i = 0; i < vec.size(); i++) {
        if (vec[i] == vec[i + 1] - 1) {
            int inicio, fin;
            inicio = vec[i];
            fin = inicio;
            i++;
            while (i < vec.size() && fin + 1 == vec[i]) {
                fin++;
                i++;
            }

            extra += to_string(inicio) + "-" + to_string(fin);
        }
        else {
            extra += to_string(vec[i]);
        }

        if (i < vec.size() - 1) {
            extra += ", ";
        }
        else if (i == vec.size() - 2) {
            extra += " y ";
        }
    }

    return extra;
}

/*
    Función para hacer el reemplazo de páginas
    Parámetros:
    - posicion (int): posicion en S de la página de un proceso a la cual se le quiere asignar algún marco de página
*/
void swapping(int posicion) {

    cantSwaps++;
    tiempo++;

    double valor = INT_MAX;
    int posReal;

    //Buscamos el valor más chico de timestamp
    for (int i = 0; i < 128; i++) {
        if (M[i] != NULL && valor > M[i]->timestamp) {
            valor = M[i]->timestamp;
            posReal = i;
        }
    }

    int proceso = M[posReal]->idProceso;

    for (int i = 0; i < 256; i++) {
        if (S[i] != NULL && S[i]->idProceso == proceso && S[i]->marcoDePagina == posReal) {
            S[i]->marcoDePagina = -1;

            S[posicion]->marcoDePagina = posReal;

            M[posReal]->idProceso = S[posicion]->idProceso;
            M[posReal]->timestamp = tiempo;
            M[posReal]->cantBytes = S[posicion]->cantBytes;

            cout << "La pagina " << S[posicion]->pagina << " del proceso " << S[posicion]->idProceso << " fue swappeada al marco " << S[posicion]->marcoDePagina << endl;
        }
    }
}

/*
    Función utilizada para cargar un procesos a memoria
    Parámetros:
        -bytes: representa el tamaño del proceso
        -proceso: representa el ID del proceso
 */
void cargarAMemoria(int bytes, int proceso) {

    //cout << "Cargando memoria de " << proceso << endl;

    if (bytes <= 2048) {

        int cantPaginas = ceil(bytes / tamPagina);

        if (cantPaginasVirtLibres >= cantPaginas) {
            Proceso proc;
            bool paginaEncontrada;
            int bytesExtra = bytes;

            for (int i = 0; i < cantPaginas; i++) {
                paginaEncontrada = false;

                for (int j = 0; !paginaEncontrada && j < 256; j++) {
                    if (S[j] == NULL) {
                        S[j] = new ProcesoVirtual;
                        S[j]->idProceso = proceso;
                        S[j]->timestamp = ++tiempo;
                        S[j]->pagina = i;

                        cantPaginasVirtLibres--;

                        paginaEncontrada = true;

                        // conseguir marco de pagina
                        bool memoriaEncontrada = false;
                        for (int j = 0; !memoriaEncontrada && j < M.size(); j++) {
                            if (M[j] == NULL) {
                                M[j] = new ProcesoReal;
                                M[j]->idProceso = proceso;
                                M[j]->timestamp = tiempo;
                                M[j]->cantBytes = (bytesExtra > tamPagina) ? tamPagina : bytesExtra;

                                S[i]->cantBytes = (bytesExtra > tamPagina) ? tamPagina : bytesExtra;
                                S[i]->marcoDePagina = j;

                                bytesExtra -= tamPagina;

                                memoriaEncontrada = true;
                            }
                        }

                        if (!memoriaEncontrada) {
                            swapping(j);
                        }
                    }
                }
            }

            proc.idProceso = proceso;
            proc.cantPaginas = cantPaginas;
            proc.tiempoInicio = tiempo;
            proc.tamProceso = bytes;
            proc.tiempoFinal = -1;
            proc.cantPageFaults = 0;

            procesos.push_back(proc);

            vector<int> marcos;

            for (int i = 0; i < 128; i++) {
                if (M[i] != NULL && proceso == M[i]->idProceso) {
                    marcos.push_back(i);
                }
            }

            cout << "Se asignaron los marcos de pagina [" << mostrarRangos(marcos) << "]" << endl;
        }
        else {
            cout << "ERROR: No cabe el proceso en memoria virtual\n";
        }
    }
    else {
        cout << "ERROR: No cabe el proceso en memoria real\n";
    }
}

/*
    Funcion utilizada para acceder a una direccion de memoria de un proceso dado
    Parámetros:
        -direccion: Direccion virtual
        -proceso: ID del proceso
        -modificar: condicion para saber si fue modificado
*/
void accederADireccion(int direccion, int proceso, bool modificar) {
    cout << "Obtener la direccion real correspondiente a la direccion virtual " << direccion << " del proceso " << proceso << endl;
    tiempo += 0.1;

    for(int i = 0; i < procesos.size(); i++){
        //Recorrer los procesos hasta encontrar el indicado con ID
        if(proceso == procesos[i].idProceso){
            //Calcular pagina
            int pagina = (direccion / tamPagina) - ((direccion % tamPagina == 0 && direccion != 0) ? 1 : 0);

            //Recorrer las paginas hasta que encontremos la pagina donde esta el proceso
            for(int j = 0; j < 256; j++){
                if(S[j] != NULL && S[j]->idProceso == proceso && S[j]->pagina == pagina){

                    //Checamos si el marco de página [MODIFICAR COMENT]
                    if(S[j]->marcoDePagina == -1){
                        swapping(j);
                        procesos[i].cantPageFaults++;
                    }

                    int dirReal = direccion % tamPagina + S[j]->marcoDePagina * tamPagina + ((direccion % tamPagina == 0 && direccion != 0) ? tamPagina : 0);
                    
                    //Ajustar cambios en memoria conforme a LRU
                    if(politica == "LRU"){
                        M[S[j]->marcoDePagina]->timestamp = tiempo;
                    }

                    cout << "Direccion Virtual es = " << direccion << " y direccion real = " << dirReal << endl;
               
                    return;
                }
            }
        }
    }
    cout << "ERROR: No se encontro la direccion de memoria dada\n";
}

/*
    Funcion utilizada para liberar el proceso de memoria
    Parámetros:
        -Proceso: Representa el ID del proceso a liberar
*/
void liberarProceso(int proceso) {
    //Utilizado para registrar los marcos de pag. a liberar
    vector <int> marcosDePagina;

    //Utilizado para registrar las pag. a liberar
    vector <int> paginas;

    //Desplegar marcos de pagina ocupaos por el proceso
    cout << "Liberar los marcos de página ocupados por el proceso " << proceso << endl;
    
    //Ciclo para liberar los marcos de página ocupados por el proceso en la memoria real
    for(int i = 0; i < 128; i++){
        if(M[i] != NULL && proceso == (M[i]->idProceso)){
            marcosDePagina.push_back(i);
            tiempo += 0.1;
            delete M[i];
            M[i] = NULL;
            
        }
    }
 
    //Ciclo para liberar las páginas ocupadas por el proceso en la memoria virtual     
    for(int i = 0; i < 256; i++){
        if(S[i] != NULL && proceso == S[i]->idProceso){
            paginas.push_back(i);
            tiempo += 0.1;
            delete S[i];
            S[i] = NULL;
            cantPaginasVirtLibres++;
        }
    }

    //Ciclo para buscar el proceso que se acaba de liberar y marcar que ya terminó  
    for(int i = 0; i < procesos.size(); i++){
        if(proceso == procesos[i].idProceso){
            procesos[i].tiempoFinal = tiempo;
        }
    }

    if(marcosDePagina.size() > 0){
        cout << "Se liberan los marcos de memoria real: [" << mostrarRangos(marcosDePagina) << "]" << endl;
    }
    else{
        cout << "El proceso no está ocupado en ningún marco de página\n";
    }

    if(paginas.size() > 0){
        cout << "Se liberan los marcos del área de swapping: [" << mostrarRangos(paginas) << "]" << endl;
    }
    else{
        cout << "El proceso no está ocupando ninguna página\n";
    }
}

/* Funcion utilizada */
void finCiclo() {

    if (procesos.size() > 0) {
        double turnaroundProm = 0;

        //El tiempo final es de -1
        for (int i = 0; i < procesos.size(); i++) {
            if (procesos[i].tiempoFinal == -1) {
                procesos[i].tiempoFinal = tiempo;
            }
            cout << "Turnaround del proceso " << procesos[i].idProceso << " es = " << procesos[i].tiempoFinal - procesos[i].tiempoInicio << endl;
            turnaroundProm += (procesos[i].tiempoFinal - procesos[i].tiempoInicio);
        }

        turnaroundProm /= double(procesos.size());
        cout << "Turnaround promedio es = " << turnaroundProm << endl;

        for (int i = 0; i < procesos.size(); i++) {
            cout << "La cantidad de Page Faults del proceso " << procesos[i].idProceso << " es = " << procesos[i].cantPageFaults << endl;
        }

        cout << "La cantidad de swaps es = " << cantSwaps << endl;
    }
    else {
        cout << "ERROR: No se ha cargado a memoria ningun proceso\n";
    }
}

/*
    Funcion utilizada para procesar la entrada, y saber que instrucción se quiere ejecutar
    Parámetros:
        -línea: La instrucción a procesar
    Output:
        -bool: Especifíca si ya se terminó el proceso
*/
bool parsearInput(string linea) {
    stringstream ss;
    string extra;
    string n, p, d, m;
    char c;
    bool valor = true; // para saber si seguir el programa o no

    ss << linea;
    ss >> c;

    cout << linea << endl; 

    switch (c) {
    case 'P':
        ss >> n >> p;
        cargarAMemoria(stoi(n), stoi(p));
        break;
    case 'A':
        ss >> d >> p >> m;
        accederADireccion(stoi(d), stoi(p), stoi(m));
        break;
    case 'L':
        ss >> p;
        liberarProceso(stoi(p));
        break;
    case 'C':
        /*getline(ss, extra);
        cout << extra << endl;*/
        break;
    case 'F':
        finCiclo();
        reiniciarValores();
        break;
    case 'E':
        //Suena dubstep en el fondo
        cout << "aZtaaaa la procSimaaaaaa" << endl;

        valor = false;
        break;
    default:
        cout << "ERROR: Instrucción invalida\n";
        break;
    }
    
    cout << endl;

    return valor;
}

/*Funcion principal para la lectura del archivo y reinicio del proceso*/
int main() {

    ifstream entrada;
    string linea, nombreArch;
    bool seguir;

    cout << "Nombre archivo\n";
    cin >> nombreArch;

    valoresIniciales();

    entrada.open(nombreArch);

    if (entrada.is_open()) {
        cout << "LRU\n";
        politica = "LRU";
        do {
            getline(entrada, linea);
            seguir = parsearInput(linea);
        } while (seguir);
    }

    entrada.close();

    reiniciarValores();

    entrada.open(nombreArch);

    if (entrada.is_open()) {
        cout << "FIFO\n";
        politica = "FIFO";
        do {
            getline(entrada, linea);
            seguir = parsearInput(linea);
        } while (seguir);
    }

    entrada.close();
    
    return 0;
}
