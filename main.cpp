#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
using namespace std;

// класс ячейки
class cell{
public:
    // является ли ячейка препятствием
    bool occupancy;
    // покрыта ли ячейка
    bool coverage = false;
    // очередность во время покрытия
    string coverage_index;
    // расстояние от начальной ячейки
    int distance;
    // переменная для поиска в ширину
    bool used;

    cell(bool o){
        occupancy = o;
        if(occupancy) {coverage_index = "#"; coverage = true; distance = -1;}
        else coverage_index = "0";
    };
    // словарь соседей
    map <string, cell *> neighbours = {{"down", nullptr}, {"up", nullptr}, {"left", nullptr}, {"right", nullptr}};  
};

class coverage{
private:
    // вектор с ячейками
    vector <vector<cell>> map_data;
    int n, m;
    // функция проверки того, покрыта ли карта
    bool iscoverage(vector <vector<cell>> map_data){
        for(int i = 0; i < map_data.size(); i++){
            for(int j = 0; j < map_data[0].size(); j++){
                if (!map_data[i][j].coverage){
                    return false;
                } 
            }
        }
        return true;
    }
    // запись в файлы расстояний от начальной ячейки
    void distance_map_to_txt(){
        ofstream fout("distance_map.txt");
        for (int i = 0; i < n; i++){
            for (int j = 0; j < m; j++){
                fout << map_data[i][j].distance << '\t';
            }
            fout << endl;
        }
    }

    // запись карты покрытия в файл
    void coverage_map_to_txt(){
        ofstream fout("coverage_map.txt");
        for (int i = 0; i < n; i++){
            for (int j = 0; j < m; j++){
                fout << map_data[i][j].coverage_index << '\t';
            }
            fout << endl;
        }
    }
public:
    // конструктор считывает файл с картой и создает вектор ячеек
    coverage(string filename){
        ifstream file;
        file.open(filename);
        string a;
        // во время считывания файла определяем занятость ячейки
        while (getline(file, a))
        {
            vector <cell> line;    
            for(int s = 0; s < a.length(); s++){
                if(a[s] != ' ' && a[s]!='\n'){
                    cell c(a[s] == '#');
                    line.push_back(c);
                }
            }
            map_data.push_back(line);
        }

        // n - количество строк, m - количество столбцов
        n = map_data.size(); m = map_data[0].size();
        // определяем соседей ячеек
        for(int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++){
                if (i != 0 && !map_data[i-1][j].occupancy) map_data[i][j].neighbours["up"] = &map_data[i-1][j];
                if (i != n-1 && !map_data[i+1][j].occupancy) map_data[i][j].neighbours["down"] = &map_data[i+1][j];
                if (j != 0 && !map_data[i][j-1].occupancy) map_data[i][j].neighbours["left"] = &map_data[i][j-1];
                if (j != m-1 && !map_data[i][j+1].occupancy) map_data[i][j].neighbours["right"] = &map_data[i][j+1];
            }
        }

        // предполагается, что начальная ячейка находится в верхнем левом углу
        queue <cell *> used;
        map_data[0][0].used = true;
        map_data[0][0].distance = 0;
        used.push(&map_data[0][0]);
        // метод поиска в ширину
        while (!used.empty())
        {
            map <string, cell*> :: iterator it = used.front()->neighbours.begin();
            for (int i = 0; it != used.front()->neighbours.end(); it++, i++){
                if (it->second != nullptr && !it->second->used){
                    it->second->used = true;
                    it->second->distance = used.front()->distance + 1;
                    used.push(it->second);
                }
            }
            used.pop();
        }

        distance_map_to_txt();
    }
    // функция, определяющая траекторию покрытия карты
    void coverage_planning(){
        // переменная текущей ячейки
        cell *cur_cell = &map_data[0][0];
        // счетчик очередности ячеек
        int counter = 0;
        cur_cell->coverage = true;

        while (!iscoverage(map_data)){
            // выбираем непокрытого соседа, расстояние которого от начальной ячейки минимально
            cell *min_neigh = nullptr;
            map <string, cell*> :: iterator it = cur_cell->neighbours.begin();
            string key;
            for (int i = 0; it != cur_cell->neighbours.end(); it++, i++){
                if (min_neigh != nullptr){
                    if (it->second != nullptr && min_neigh->distance > it->second->distance && !it->second->coverage) 
                    {
                        min_neigh = it->second;
                        key = it->first;
                    }
                }
                else if (it->second != nullptr && !it->second->coverage) 
                {
                    min_neigh = it->second;
                    key = it->first;
                }
            }
            
            // в случае, если все соседи вокруг уже покрыты, ищем ближайшую непокрытую ячейку с помощью поиска в ширину
            if (min_neigh == nullptr){
                queue <cell*> used;
                used.push(cur_cell);
                while (!used.empty() && min_neigh == nullptr){
                    map <string, cell*> :: iterator it = used.front()->neighbours.begin();
                    for (int i = 0; it != used.front()->neighbours.end(); it++, i++){
                        if (it->second != nullptr && it->second->coverage){
                            used.push(it->second);
                        }
                        else if (it->second != nullptr){
                            min_neigh = it->second;
                            break;
                        }
                    }
                    used.pop();
                }
            }
            
            counter++;
            cur_cell->coverage_index = to_string(counter);
            cur_cell->coverage = true;
            cur_cell = min_neigh;
        }
        coverage_map_to_txt();
    }
};

int main(){    
    coverage cov("map.txt");
    cov.coverage_planning();
}