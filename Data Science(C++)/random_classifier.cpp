#include <cerrno>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iterator>
#include <string>
#include <algorithm>
#include <random>
#include <iostream>
#include <map>
#include <thread>
#include <unordered_set>
#include <list>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

enum columnType{inputCol,outputCol};
enum columnDataType{integerData,numericData,atrib};

static inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}


static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}


static inline void trim(std::string &s)
{
    if(isspace(s[s.length()-1]))
        s.pop_back();
    /*
    ltrim(s);
    rtrim(s);
    */
}

class DataException : public std::exception
{
public:
    DataException(std::string message) : std::exception(), message(message)
    {
    }

    const char *what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};


class DataRow
{
    public:
    DataRow(const int size,const std::string* values)
    :size(size)
    {
        this->values = new std::string[size];
        for(int i=0;i<size;i++)
        {
            this->values[i] = values[i];
        }
    }

    DataRow(const DataRow& dataRow)
    :size(dataRow.size)
    {
        this->values = new std::string[size];
        for(int i=0;i<size;i++)
        {
            this->values[i] = dataRow.values[i];
        }
    }

    std::string operator[](int index) const
    {
        if(values==NULL)
            {
                throw std::exception();
            }
        if(index>size)
            throw "Index Out of Bounds";
        return values[index];
    }

    std::string toString()
    {
        if(values==NULL)
            std::cout<<"Invalid";
        std::string s = "";
        if(size>0)
        {
            s = values[0];
        }
        for(int i=1;i<size;i++)
        {
            s+=","+values[i];
        }

        return s;
    }

    ~DataRow()
    {
        if(values!=NULL){
            delete[] values;
            values = NULL;
        }
    }
    
    private:
    std::string *values;
    int size;
};


struct ColomnMeta
{
    std::string label;
    columnType type;
    columnDataType dataType; 
    std::pmr::map<std::string,int> table;
};


class DataSet
{
    public:
    DataSet** split(int index,std::pmr::unordered_set<std::string> comparison_set) const
    {   
        DataSet** newSets = new DataSet*[2];
        newSets[0] = new DataSet(this->columns,this->meta);
        newSets[1] = new DataSet(this->columns,this->meta);

        for(auto it=this->rows.begin();it!=this->rows.end();it++)
        {
            if(comparison_set.count((*it)[index])==0)
            {
                newSets[1]->rows.push_front(*it);
            }
            else 
            {
                newSets[0]->rows.push_front(*it);
            }
        }
        return newSets;
    }

    DataSet** trainTestSplit(double train_sample_size_percentage)
    {
        DataSet** newSets = new DataSet*[2];
        newSets[0] = new DataSet(this->columns,this->meta);
        newSets[1] = new DataSet(this->columns,this->meta);
        std::default_random_engine generator;
        std::uniform_real_distribution<double> distribution(0.0,1.0);

        for(auto it=this->rows.begin();it!=this->rows.end();it++)
        {
            double val = distribution(generator);
            if(val<train_sample_size_percentage)
            {
                newSets[0]->rows.push_front(*it);
            }
            else 
            {
                newSets[1]->rows.push_front(*it);
            }
        }
        return newSets;
    }

    std::string getOutput() const 
    {
        if(rows.size()<1)
        {   
            return "";
        }
        for(int i=0;i<columns;i++)
        {
            if(this->meta[i].type==outputCol)
                return (*rows.begin())[i];
        }
        return "";
    }

    void show()
    {
        for(int i=0;i<columns;i++)
        {
            std::cout<<meta[i].label<<",";
        }
        for(auto it=rows.begin();it!=rows.end();it++)
        {
            std::cout<<it->toString()<<"\n";
        }
    }
    DataRow operator[](int index)
    {
        if(index>this->rows.size())
            throw "Index out of bounds";
        auto it = rows.begin();
        for(int i=0;i<index;i++)
            it++;
        return *it;
    }
    DataSet(const int column_count,const ColomnMeta* meta)
    :columns(column_count)
    {
        this->meta = new ColomnMeta[columns];
        for(int i=0;i<column_count;i++)
        {
            this->meta[i] = meta[i];
        }
    }

    DataSet(const DataSet& dataSet):
    columns(dataSet.columns)
    {
        this->meta = new ColomnMeta[columns];
        for(int i=0;i<columns;i++)
        {
            this->meta[i] = dataSet.meta[i];
        }
        this->rows = dataSet.rows; 
    }

    void addData(std::string filename)
    {
        std::ifstream f(filename);
        while(!f.eof())
        {
            std::string unformatted;
            std::getline(f,unformatted);
            std::istringstream tokener(unformatted);
            std::string* values = new std::string[columns];
            for(int i=0;i<columns;i++)
            {
                std::string formatted; 
                std::getline(tokener, formatted, ',');
                trim(formatted);
                values[i] = formatted;
            }
            rows.push_front(DataRow(columns,values));
            delete[] values;
        }
        f.close();
    };

    ~DataSet()
    {
        if(meta)
        {
            delete[] meta;
            meta = NULL;
        }
    }

    int getSampleSize()
    {
        return rows.size();
    }

    std::list<DataRow>::iterator begin()
    {
        return rows.begin();
    }
    std::list<DataRow>::iterator end()
    {
        return rows.end();
    }

    private:
    friend class RandomForest;
    ColomnMeta* meta;
    std::list<DataRow> rows;
    int columns;
    
};

struct SplitSets
{
    std::pmr::unordered_set<std::string>* sets=NULL;
    std::pmr::unordered_set<std::string> output;
    int output_index;
    int count;

    void pushReverse(int j,int i,int med,int setIndex,std::pmr::unordered_set<std::string>::iterator it,std::pmr::unordered_set<std::string>::iterator end,SplitSets* splitLeft,SplitSets* splitRight)const
    {
        if(it!=end)
        {

        }
        else {
            return;
        }
        
        std::string value = *it;
        pushReverse(j+1,i,med,setIndex,++it,end,splitLeft,splitRight);
    
        if(i!=setIndex || j<=med)
        {
            splitLeft->sets[i].insert(value);
        }
        if(i!=setIndex || j>med)
        {
            splitRight->sets[i].insert(value);
        }
    }

    SplitSets** split(int setIndex,int med) const
    {
        SplitSets** splits= new SplitSets*[2];
        splits[0] = new SplitSets;
        splits[1] = new SplitSets;

        splits[0]->count = count;
        splits[1]->count = count;

        splits[0]->sets = new std::pmr::unordered_set<std::string>[count];
        splits[1]->sets = new std::pmr::unordered_set<std::string>[count];

        splits[0]->output = this->output;
        splits[1]->output = this->output;

        splits[0]->output_index = this->output_index;
        splits[1]->output_index = this->output_index;

        for(int i=0;i<count;i++)
        {
            pushReverse(0,i,med,setIndex,sets[i].begin(),sets[i].end(),splits[0],splits[1]);
        }

        return  splits;
    }

    void show() const
    {   
        for(int i=0;i<count;i++)
        {
        std::cout<<"[";
        for(auto it=sets[i].begin();it!=sets[i].end();it++)
        {
            std::cout<<*it<<" ";
        }
        std::cout<<"]\n";
        }
    };
    ~SplitSets()
    {
        if(sets!=NULL)
            delete[] sets;
    }
};

class Classifier_Node
{
    public:

    static float decodify(std::string str,columnDataType type,const std::string* listOfValues,int size)
    {
        if(type==atrib)
        {
            for(int i=0;i<size;i++)
            {
                if(str==listOfValues[i])
                    return i;
            }
            std::cout<<"Atribute couldn t be decodified\n";
            return 0;
        }
        else
        {
            try
            {
                float a = std::stof(str);
                return a;
            }
            catch(std::exception e)
            {
                std::cout<<"Numeric Value couldn t be decodified\n";
                return 0;
            }
        }
    }

    float getEntropy(int* stateCounts,int nrOfStates,float sum)
    {
        if(sum == 0)
            return 2000;
        float entropy = 0.0f;
        for(int i=0;i<nrOfStates;i++)
        {
            float prob = float(stateCounts[i])/sum;
            if(prob!=0)
            {
            entropy -= prob*(std::log(prob)/std::log(nrOfStates));
            }
        }

        return entropy;
    }

    int getIndex(std::string str,const std::string* output_values,int size)
    {
        for(int i=0;i<size;i++)
        {
            if(str==output_values[i])
                return i;
        }

        std::cout<<"Index not found\n";
        return 0;
    }

    float test(std::vector<std::string> data,const std::string*& listOfValues,int list_size,std::vector<std::string>output,const std::string* outputValues,int output_size,columnDataType type,float median,float& entropyL,float&entropyR)
    {
        int *statesL = new int[output_size];
        int *statesR = new int[output_size];
        for(int i=0;i<output_size;i++)
        {
            statesL[i] = 0;
            statesR[i] = 0;
        }

        int countL = 0;
        int countR = 0;

        auto it1 = data.begin();
        auto it2 = output.begin();
        while(it1!=data.end() && it2!=output.end())
        {
            if(decodify(*it1,type,listOfValues,list_size)<=median)
            {
                statesL[getIndex(*it2,outputValues,output_size)]++;
                countL++;
            }
            else 
            {
                statesR[getIndex(*it2,outputValues,output_size)]++;
                countR++;
            }
            it1++;
            it2++;
        } 

        entropyL = getEntropy(statesL,output_size,countL);
        entropyR = getEntropy(statesR,output_size,countR);

        return this->entropy - float(countL)/float(data.size())*entropyL
                             - float(countR)/float(data.size())*entropyR;
    }

    std::pair<float,float> getBestGini(float oldGini,float left,float right,float step,std::vector<std::string> data,const std::string*& listOfValues,int list_size,std::vector<std::string>output,const std::string* outputValues,int output_size,columnDataType type,float& entropyL,float&entropyR)
    {
        float savedMed = left; 
        for(float med = left;med<=right-step;med++)
        {
            float newGini = test(data,listOfValues,list_size,output,outputValues,output_size,type,med,entropyL,entropyR);
            if(newGini>oldGini)
            {
                savedMed = med;
                oldGini = newGini;
            }
        }
        return {oldGini,savedMed};
    }

    int stop(float left1,float right1,float step1,float left2,float right2,float step2,std::vector<std::string> data1)
    {
                
        if(data1.empty())
            return 2;
        if(this->entropy==0)
            return 1;
        if(right1-left1<step1 && right2-left2<step2)
            return 1;
        if(data1.size()<=minLeaves)
            return 1;

        return 0;
    }

    std::string getMostProbably(std::vector<std::string> output,const std::string* outputValues,int stateNr)
    {
        int *states = new int[stateNr];
        for(int i=0;i<stateNr;i++)
        {
            states[i]=0;
        }
        for(auto it=output.begin();it!=output.end();it++)
        {
            states[getIndex(*it,outputValues,stateNr)]++;
        }

        int saved=0;
        int count=0;
        for(int i=0;i<stateNr;i++)
        {
            if(states[i]>count)
                {
                    saved = i;
                    count = states[i];
                }
        }
        return outputValues[saved];
    }

    Classifier_Node(int minRowsPerLeaf,int mode,std::vector<std::string> data1,std::vector<std::string> data2,std::vector<std::string> output,float entropy,float left1,float right1,const std::string* list1,int list_size1,columnDataType type1,float step1,float left2,float right2,const std::string*list2,int list_size2,columnDataType type2,float step2,const std::string* outputValues,int output_size)
    {
        this->minLeaves = minRowsPerLeaf;
        this->entropy = entropy;    
        int check = stop(left1,right1,step1,left2,right2,step2,data1);

        if(check==1)
        {
            this->value=getMostProbably(output,outputValues,output_size);
            this->left=NULL;
            this->right=NULL;
            return;
        }

        if(check==2)
        {
            this->value = outputValues[0];
            this->left = NULL;
            this->right = NULL;
            std::cout<<"You have messed up\n";
            return;
        }

        float entropyL = 2000;
        float entropyR = 2000;
        float newEntropyL = 0;
        float newEntropyR = 0;
        float gini =0;

        auto res1 = getBestGini(0,left1,right1,step1,data1,list1,list_size1,output,outputValues,output_size,type1,entropyL,entropyR);
        auto res2 = res1;
        
        if(mode==2)
            res2= getBestGini(0,left2,right2,step2,data2,list2,list_size2,output,outputValues,output_size,type2,newEntropyL,newEntropyR);

        if(res1.first<res2.first)
        {
            entropyL = newEntropyL;
            entropyR = newEntropyR;
            this->median = res2.second;
            this->which = 1;
            gini = res1.first;
        }
        else 
        {
            this->median = res1.second;
            this->which = 0;   
            gini = res2.first;    
        }
        

        std::vector<std::string> newData1Left;
        std::vector<std::string> newData1Right;
        std::vector<std::string> newData2Left;
        std::vector<std::string> newData2Right;
        std::vector<std::string> outputLeft;
        std::vector<std::string> outputRight;
        auto it1 = data1.begin();
        auto it2 = data2.begin();
        auto it3 = output.begin();


        while(it1!=data1.end())
        {
            float a;
            if(which)
            {
                a = Classifier_Node::decodify(*it2,type2,list2,list_size2);
            }
            else 
            {
                a = Classifier_Node::decodify(*it1,type1,list1,list_size1);
            }

            if(a<=median)
            {
                newData1Left.push_back(*it1);
                newData2Left.push_back(*it2);
                outputLeft.push_back(*it3);
            }
            else
            {
                newData1Right.push_back(*it1);
                newData2Right.push_back(*it2);
                outputRight.push_back(*it3);
            }
            it1++;
            it2++;
            it3++;
        }

        if(newData1Left.empty() || newData2Left.empty() ||entropyL == 2000 || entropyR == 2000)
        {
            this->value=getMostProbably(output,outputValues,output_size);
            this->left=NULL;
            this->right=NULL;
            return;
        }

        if(which)
        {
            this->left = new Classifier_Node(minRowsPerLeaf,mode,newData1Left,newData2Left,outputLeft,entropyL,left1,right1,list1,list_size1
        ,type1,step1,left2,median,list2,list_size2,type2,step2,outputValues,output_size);
            this->right = new Classifier_Node(minRowsPerLeaf,mode,newData1Right,newData2Right,outputRight,entropyR,left1,right1,list1,list_size1
        ,type1,step1,median+step2,right2,list2,list_size2,type2,step2,outputValues,output_size);
        }
        else 
        {
            this->left = new Classifier_Node(minRowsPerLeaf,mode,newData1Left,newData2Left,outputLeft,entropyL,left1,median,list1,list_size1
        ,type1,step1,left2,right2,list2,list_size2,type2,step2,outputValues,output_size);
            this->right = new Classifier_Node(minRowsPerLeaf,mode,newData1Right,newData2Right,outputRight,entropyR,median+step1,right1,list1,list_size1
        ,type1,step1,left2,right2,list2,list_size2,type2,step2,outputValues,output_size);
        }

    }
    ~Classifier_Node()
    {

    }
    private:
    friend class Classifier_Tree;
    float minLeaves;
    float entropy;
    std::string value;
    float median;
    bool which;
    Classifier_Node* left;
    Classifier_Node* right;
};

class Classifier_Tree
{
    public:

    Classifier_Tree(int mode,int minLeaves,const std::string* listOfValues1,int list1_size,columnDataType type1,float step1,const std::string* listOfValues2,int list2_size,columnDataType type2,float step2,const std::string* outputValues,int output_size,columnDataType output_type)
    {
        this->listOfValues1 = NULL;
        this->listOfValues2 = NULL;
        this->step1 = step1;
        this->step2 = step2;
        this->minLeaves = minLeaves;
        this->list_size1 = list1_size;
        this->list_size2 = list2_size;
        this->output_size = output_size;
        this->type1 = type1;
        this->type2 = type2;
        this->output_type = output_type;
        this->mode = mode;

        if(listOfValues1!=NULL && list1_size>0)
        {
            this->listOfValues1 = new std::string[list1_size];
            for(int i=0;i<list1_size;i++)
                this->listOfValues1[i] = listOfValues1[i];
        }

        if(listOfValues2!=NULL && list2_size>0)
        {
            this->listOfValues2 = new std::string[list2_size];
            for(int i=0;i<list2_size;i++)
                this->listOfValues2[i] = listOfValues2[i];
        }

        if(outputValues!=NULL && output_size>0)
        {
            this->output_list = new std::string[output_size];
            for(int i=0;i<output_size;i++)
                this->output_list[i] = outputValues[i];
        }
        else 
            throw  DataException("Classification Tree: Missing output");

    }

    void addValues(DataRow row,int index1,int index2,int output_index)
    {
        this->data1.push_back((row)[index1]);
        this->data2.push_back((row)[index2]);
        this->output.push_back((row)[output_index]);
    }

    std::string predict(DataRow row,int index1,int index2,int output_index)
    {
        
        Classifier_Node* temp = entry;
        
        while(temp->left!=NULL && temp->right!=NULL)
        {
            float a;
            if(temp->which)
                a = Classifier_Node::decodify(row[index2],type2,listOfValues2,list_size2);
            else
                a = Classifier_Node::decodify(row[index1],type1,listOfValues1,list_size1);
            if(a<=temp->median)
                temp = temp->left;
            else
                temp = temp->right;
        }
        
        return temp->value;
        
    }
    
    void train()
    {
        float left1;
        float left2;
        float right1;
        float right2;

        if(type1==atrib)
        {
            left1 = 0;
            right1 = list_size1-1;
        }
        else 
        {
            right1 = -MAXFLOAT;
            left1 = MAXFLOAT;
            for(auto it = data1.begin();it!=data1.end();it++)
            {
                float num = Classifier_Node::decodify(*it, type1,listOfValues1,list_size1);
                if(left1>num)
                {
                    left1 = num;
                }
                if(right1<num)
                {
                    right1 = num;
                }
            }
        }

        if(type2==atrib)
        {
            left2 = 0;
            right2 = list_size2-1;
        }
        else 
        {
            right2 = -MAXFLOAT;
            left2 = MAXFLOAT;
            for(auto it = data2.begin();it!=data2.end();it++)
            {
                float num = Classifier_Node::decodify(*it, type2,listOfValues2,list_size2);
                if(left2>num)
                {
                    left2 = num;
                }
                if(right2<num)
                {
                    right2 = num;
                }
            }
        }

        this->entry = new Classifier_Node(minLeaves,mode,data1,data2,output,1,left1,right1,listOfValues1,list_size1
        ,type1,step1,left2,right2,listOfValues2,list_size2,type2,step2,output_list,output_size);
    }

    ~Classifier_Tree()
    {
        if(listOfValues1!=NULL)
        {
            delete [] listOfValues1;
        }
        if(listOfValues2!=NULL)
        {
            delete [] listOfValues2;
        }
    };  
    private:
    int mode;
    int minLeaves;
    int list_size1;
    int list_size2;
    int output_size;
    float step1;
    float step2;
    std::string* listOfValues1;
    std::string* listOfValues2;
    std::string* output_list;
    columnDataType type1;
    columnDataType type2;
    columnDataType output_type;
    Classifier_Node* entry;
    std::vector<std::string> data1;
    std::vector<std::string> data2;
    std::vector<std::string> output;
};

class RandomForest
{
    public:

    int getIndex(std::string str,std::string* outputList,int size)
    {
        int i=0;
        for(int i=0;i<size;i++)
        {
            if(str==outputList[i])
                return i;
        }
        std::cout<<"String not found in output list\n";
        return 0;
    }

    void showConfussion_Matrix(DataSet *s,std::string* outputList,int size,int output_index)
    {
        int** mat = new int*[size];
        for(int i=0;i<size;i++){
            mat[i] = new int[size];
            for(int j=0;j<size;j++)
            {
                mat[i][j]=0;
            }
        }
            for(auto it=s->rows.begin();it!=s->rows.end();it++)
        {
            std::string res = predict(*it,output_index);
            mat[getIndex((*it)[output_index],outputList,size)][getIndex(res,outputList,size)]++;
        }

        for(int i=0;i<size;i++)
        {   for(int j=0;j<size;j++)
            {
                std::cout<<mat[i][j]<<" ";
            }
            std::cout<<"\n";
        }
    }
    float getAccuracy(const DataSet *s,int output_index)
    {   
        int predicted = 0;
        for(auto it=s->rows.begin();it!=s->rows.end();it++)
        {
            std::string res = predict(*it,output_index);
            if((*it)[output_index]==res)
                predicted++;
        }
        return float(predicted)/float(s->rows.size());
    }
    static void dummyFunction(int index1, int index2, int output_index, DataSet* set, Classifier_Tree*& tree,
                              int mode, int minLeaves, std::string* listOfValues1, int list1_size, columnDataType type1, 
                              float step1, std::string* listOfValues2, int list2_size, columnDataType type2, 
                              float step2, std::string* outputValues, int output_size, columnDataType output_type)
    {

        tree = new Classifier_Tree(mode, minLeaves, listOfValues1, list1_size, type1, step1, listOfValues2, list2_size, type2, step2, outputValues, output_size, output_type);
        for(auto it = set[0].begin(); it != set[0].end(); it++)
        {
            tree->addValues(*it, index1, index2, output_index);
        }
        tree->train();

    }
    
    RandomForest(DataSet* s,int* sizes,int*steps,std::string** references,int* order,int minLeaves,std::string* output_values,int output_size)
    {
        int a = (s->columns-1)/2;
        int m = (s->columns-1)%2;
        
        std::vector<Classifier_Tree*> trees(a+m+3, nullptr);
        std::vector<std::thread> threads;


        int j=0;
        for(int i = 0; i < s->columns-1; i+=2)
        {
                pairs.push_back(std::pair<int,int>(order[i],order[i+1]));
                threads.emplace_back(dummyFunction, order[i],order[i+1],s->columns-1, s, std::ref(trees[j]),1, minLeaves,
                references[i], sizes[i], s->meta[order[i]].dataType, steps[i], // Example step value
                references[i+1], sizes[i+1], s->meta[order[i+1]].dataType, steps[i+1], // Example step value
                output_values, output_size, atrib); // Example columnDataType
                j++;
        }
       
       
        if(m==1)
        {
            
            int i = s->columns-2;
            pairs.push_back(std::pair<int,int>(order[i],order[i]));
            threads.emplace_back(dummyFunction, order[i],order[i],s->columns-1, s, std::ref(trees[i]),2, minLeaves,
                references[i], sizes[i], s->meta[order[i]].dataType, steps[i], // Example step value
                references[i], sizes[i], s->meta[order[i]].dataType, steps[i], // Example step value
                output_values, output_size, atrib); // Example columnDataType
        }

       
        for(auto& thread : threads)
        {
            if(thread.joinable())
            {
                thread.join();
            }
        }

        this->trees = trees;
        
    };

    static void testFunc()
    {
        
    }
    static void getResult(DataRow r,int index1,int index2,int output_index,std::string &str,Classifier_Tree* tree)
    {   
      str = tree->predict(r,index1,index2,output_index);
    }

    std::string predict(DataRow row,int output_index)
    {
        std::vector<std::thread> threads;
        std::vector<std::string> results(pairs.size(),"");

        int i=0;
        for(auto it=pairs.begin();it!=pairs.end();it++)
        {
            int index1 = it->first;
            int index2 = it->second;
            threads.emplace_back(getResult,row,index1,index2,output_index,std::ref(results[i]),trees[i]);
            i++;
        }
        

        
        for(auto& thread : threads)
        {
            
            if(thread.joinable())
            {
                thread.join();
            }
        }

        std::map<std::string,int> choices;
        int num=-1;
        std::string res = "";        
        
        for(auto it=results.begin();it!=results.end();it++)
        {
            if(choices.count(*it)==0)
            {
                choices.insert(std::pair<std::string,int>(*it,0));
            }
            else 
            {
                choices.at(*it)++;
            }
        }

        for(auto it=choices.begin();it!=choices.end();it++)
        {
            if(num<(*it).second)
            {
                res=(*it).first;
                num=(*it).second;
            }
        }
        return res;
    }

    private:
    std::vector<std::pair<int,int>> pairs;
    std::vector<Classifier_Tree*> trees; 
};


int main()
{
    const ColomnMeta metas[] = 
    {
        {"price",inputCol,atrib},
        {"maintenance",inputCol,atrib},
        {"doors",inputCol,atrib},
        {"people",inputCol,atrib},
        {"luggage",inputCol,atrib},
        {"safety",inputCol,atrib},
        {"decision",outputCol,atrib}
    };

    std::string list1[] = {"vhigh","high","med","low"};
    std::string list2[] = {"vhigh","high","med","low"};
    std::string list3[] = {"2","3","4","5more"};
    std::string list4[] = {"2","4","more"};
    std::string list5[] = {"small","med","big"};
    std::string list6[] = {"low","med","high"};
    std::string outputList[] = {"unacc","acc","good","vgood"};

    std::string* references[]{list1,list2,list3,list4,list5,list6};
    int sizes[] = {4,4,4,3,3,3};
    int steps[] = {1,1,1,1,1,1};
    int order[] = {0,1,2,3,4,5};
    DataSet set(7,metas);
    set.addData("car_evaluation.csv");

    DataSet** TestTrainSet = set.trainTestSplit(0.75);

    RandomForest forest(TestTrainSet[0],sizes,steps,references,order,0,outputList,4);
    std::cout<<forest.getAccuracy(TestTrainSet[1],6)*100<<"\n";
    forest.showConfussion_Matrix(TestTrainSet[1],outputList,4,6)  
;
    return 0;
}