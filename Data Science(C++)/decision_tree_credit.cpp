#include <cmath>
#include <cstddef>
#include <exception>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <list>
#include <sys/types.h>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <random>
#include <algorithm>
#include <cctype>


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
    ltrim(s);
    rtrim(s);
}


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


struct Encodings
{
    
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
            int bad_data = 0;
            for(int i=0;i<columns;i++)
            {
                std::string formatted; 
                std::getline(tokener, formatted, ',');
                trim(formatted);
                if(formatted=="")
                    bad_data=1;
                values[i] = formatted;
            }
            if(!bad_data)
            {
            rows.push_front(DataRow(columns,values));
            }
            else {
            std::cout<<"Corrupted Data"<<"\n";
            }
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

    private:
    friend class DecisionTree;
    friend class DecisionTreeNode;
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

    SplitSets** split(int setIndex,int med,columnDataType type) const
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

        if(type==atrib)
        for(int i=0;i<count;i++)
        {
            pushReverse(0,i,med,setIndex,sets[i].begin(),sets[i].end(),splits[0],splits[1]);
        }
        
        else
        {
            auto it = this->sets[setIndex].begin();
            for(int i = 0;i<med;i++)
            {
                it++;
            }

            float trueMed;
            try{
             trueMed = std::stof(*it);
            }
            catch(std::exception e)
            {
                trueMed = 0;
                std::cout<<"Bad Conversion\n";
            }
            for(int i = 0;i<count;i++)
            {
                for(auto it =this->sets[i].begin();it!=sets[i].end();it++)
                {
                try{
                  if(i!=setIndex || std::stof(*it)<=trueMed)
                     {   
                    splits[0]->sets[i].insert(*it);
                    }
                    if(i!=setIndex || std::stof(*it)>trueMed)
                    {
                    splits[1]->sets[i].insert(*it);
                    }
                }
            catch(std::exception e)
            {
                trueMed = 0;
                std::cout<<"Bad Conversion\n";
            }
                }
            }   
        };


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

class DecisionTreeNode
{
    public:

    static std::string getResult(DecisionTreeNode* node,const SplitSets& split,const DataRow& row)
    {   
        
        if(node==NULL)
            return "";

        if(node->left == NULL || node->right == NULL)
            return node->value;
       
        std::map<std::string,float> encoddings;
        int i = 0;
       

        for(auto it=split.sets[node->column_index].begin();it!=split.sets[node->column_index].end();it++)
        {
            encoddings.insert(std::pair<std::string,float>(*it,i));
            i++;
        }

        int a;
        int b;
        if(node->type == atrib)
        {
        a = encoddings.at(row[node->column_index]);
        b = encoddings.at(node->median);
        }

        else 
        {
            a = std::stof(row[node->column_index]);
            b = std::stof(node->median);
        }

        
        if(a<=b)
            return getResult(node->left,split,row);

        return getResult(node->right,split,row);
        

    }

    float getEntropy(std::map<std::string,int> states,float sum)
    {
        if(sum==0)  
            return 2000;
        float entropy = 0;
        for(auto it=states.begin();it!=states.end();it++)
        { 
            if(it->second!=0){
            float prob = float(it->second)/sum;
            //std::cout<<it->second<<"/"<<prob<<" ";
            entropy -= prob*(std::log(prob)/log(states.size()));
            }
        }
        //std::cout<<"\n";
        return entropy;
    }

    int stop(const DataSet& set,const SplitSets& splits)
    {
        if(this->entropy == 0)
            return 1;

        if(set.rows.empty())
            return 2;

        
        
        if(set.rows.size()<=10)
            return 3;
        

        for(int i=0;i<=splits.count;i++)
        {
            if(splits.sets[i].size()>1)
                return 0;
        } 

        return 1;
    }

    float test(const DataSet& set,const SplitSets& splits,int index,float med,float& entropyL,float& entropyR)
    {   
        std::map<std::string,float> encoddings;
        std::map<std::string,int> left_val;
        std::map<std::string,int> right_val;
        float sum_left = 0;
        float sum_right = 0;
        if(set.meta[index].dataType==atrib){
        int i = 0;
        for(auto it=splits.sets[index].begin();it!=splits.sets[index].end();it++)
        {
            encoddings.insert((std::pair<std::string,float>{*it,i}));
            i++;
        }
        }
        else if(set.meta[index].dataType==integerData){
            for(auto it=splits.sets[index].begin();it!=splits.sets[index].end();it++)
            {
                encoddings.insert((std::pair<std::string,float>{*it,stoi(*it)}));   
            }
        }

        else {
            bool recalibrated = 0;
            int i = 0;
            for(auto it=splits.sets[index].begin();it!=splits.sets[index].end();it++)
            {   
                try{
                encoddings.insert((std::pair<std::string,float>{*it,stof(*it)}));
                if(i==med && !recalibrated)
                {
                    med = stof(*it);
                    recalibrated = 1;
                }
                }
                catch(std::exception e){
                encoddings.insert((std::pair<std::string,float>{*it,0}));
                std::cout<<"Corrupted data"<<"\n";
                }
                i++;
            }
        }
        
        for(auto it=splits.output.begin();it!=splits.output.end();it++)
        {
            left_val.insert((std::pair<std::string,int>{*it,0}));
            right_val.insert((std::pair<std::string,int>{*it,0}));
        }

        for(auto it = set.rows.begin();it!=set.rows.end();it++)
        {
            try{
            if(encoddings.at((*it)[index])<=med)
            {
                try{
                left_val.at((*it)[splits.output_index])++;
                sum_left++;
                }
                catch(std::exception e)
                {
                    std::cout<<(*it)[splits.output_index]<<"\n"<<"If Block"<<"\n";
                    throw "Stop If Block";
                }
            }
            else 
            {
                try{
                right_val.at((*it)[splits.output_index])++;
                sum_right++;
                }
                 catch(std::exception e)
                {
                    std::cout<<(*it)[splits.output_index]<<"\n"<<"Else Block"<<"\n";
                    throw "Stop Else Block";
                }
            }
            }
            catch(std::exception e)
            {
                    std::cout<<(*it)[index]<<"\n"<<"For Block"<<"\n";
                    throw "Stop For Block";
            }
        }
        //std::cout<<"Placement: "<<index<<" "<<med<<"\n";
        //std::cout<<"Left: "<<sum_left<<"\n";
        //std::cout<<"Values: ";
        entropyL = getEntropy(left_val,sum_left);
        //std::cout<<"Entropy: "<<entropyL<<"\n";
        //std::cout<<"Right: "<<sum_right<<"\n";
        //std::cout<<"Values: ";
        entropyR = getEntropy(right_val,sum_right);
        //std::cout<<"Entropy: "<<entropyR<<"\n";

        return this->entropy-(float(sum_left)/set.rows.size())*entropyL
                            -(float(sum_right)/set.rows.size())*entropyR;
    };

    static std::string getWorstOutcome(const DataSet& set,const SplitSets& splits)
    {
        std::map<std::string,int> encoddings;
        std::string res = *splits.output.begin();
        for(auto it=splits.output.begin();it!=splits.output.end();it++)
        {
            encoddings.insert((std::pair<std::string,int>{*it,0}));
        }
        for(auto it=set.rows.begin();it!=set.rows.end();it++)
        {
            encoddings.at((*it)[splits.output_index])++;
        }

        for(auto it=splits.output.begin();it!=splits.output.end();it++)
        {
            int num = encoddings.at(*it);
            std::cout<<*it<<" ";
            if(num!=0)
            {
                std::cout<<"\n";
                return *it;
            }
        }
        return res;

    }

    static std::string getLikelyValue(const DataSet& set,const SplitSets& splits)
    {
        std::map<std::string,int> encoddings;
        std::string res = *splits.output.begin();
        for(auto it=splits.output.begin();it!=splits.output.end();it++)
        {
            encoddings.insert((std::pair<std::string,int>{*it,0}));
        }
        int mx = 0;
        for(auto it=set.rows.begin();it!=set.rows.end();it++)
        {
            encoddings.at((*it)[splits.output_index])++;
        }

        for(auto it=splits.output.begin();it!=splits.output.end();it++)
        {
            int num = encoddings.at(*it);
            if(num>mx)
            {
                res = *it;
                mx = num;
            }
        }
        return res;
    }

    DecisionTreeNode(const DataSet& set,const SplitSets& splits,float entropy)
    {
        //std::cout<<"splitting"<<"\n";
    
        this->entropy = entropy;
        int check = stop(set,splits);

        if(check==2)
        {
            this->left = NULL;
            this->right = NULL;
            int val = rand() % splits.output.size();
            auto it = splits.output.begin();
            for(int i = 0;i< val ;i++)
            {
                it++;
            }
            this->value = *it;
            return;
        }
        if(check==1)
        {
            this->left = NULL;
            this->right = NULL;
            this->value = set.getOutput();
            return; 
        }

        if(check==3)
        {
            this->left = NULL;
            this->right = NULL;
            this->value = getLikelyValue(set,splits);
            return;
        }

        float oldGini = 0;
        int savaedIndex = 0;
        int savedValue = 0;
        float entropyL;
        float entropyR;
        
        
        for(int i=0;i<splits.count;i++)
        {
            
            for(int med = 0 ;med<(int)(splits.sets[i].size())-1;med++)
            {
                //std::cout<<med<<"\n";  
               float  newGini = test(set,splits,i,med,entropyL,entropyR);
               if(newGini>oldGini)
                    {
                        savaedIndex = i;
                        savedValue = med;
                        oldGini = newGini;
                    }
            }
           
        }
        

        this->column_index = savaedIndex;
        this->type = set.meta[savaedIndex].dataType;
        auto it = splits.sets[column_index].begin();

        int k = 0;
        while(k<savedValue)
        {
            it++;
            k++;
        }

        this->median = *it;
        

        if(entropyL>1 || entropyR>1)
        {
            this->left = NULL;
            this->right = NULL;
            this->value = getLikelyValue(set,splits);
        }
        else
        {
        SplitSets** newSplitSets = splits.split(savaedIndex,savedValue,set.meta[column_index].dataType);
        DataSet** newDateSets = set.split(savaedIndex,newSplitSets[0]->sets[savaedIndex]); 

        if(newDateSets[0]->rows.size()!=0 && newDateSets[1]->rows.size()!=0){
        this->left = new DecisionTreeNode(*newDateSets[0],*newSplitSets[0],entropyL); 
        this->right = new DecisionTreeNode(*newDateSets[1],*newSplitSets[1],entropyR);
        }

        else 
        {
            this->left = NULL;
            this->right = NULL;
            this->value = getLikelyValue(set,splits);
        }
        delete newSplitSets[0];
        delete newSplitSets[1];
        delete newDateSets[0];
        delete newDateSets[1];
        delete[] newSplitSets;
        delete[] newDateSets; 
        }

     }
    ~DecisionTreeNode()
    {
        if(left!=NULL)
            delete left;
        if(right!=NULL)
            delete right;
    }

    std::string value;
    std::string median;
    int column_index;
    columnDataType type;
    float entropy;
    void split();

    DecisionTreeNode *left;
    DecisionTreeNode *right;
};


class DecisionTree
{
    public:
    DecisionTree(const DataSet& set) : dataSet(set)
{
    
    int count = 0;
    for (int i = 0; i < dataSet.columns; i++)
    {
        count++;
    }
    splitSets.count = count-1;
    splitSets.sets = new std::pmr::unordered_set<std::string>[count];


    for (auto it = dataSet.rows.begin(); it != dataSet.rows.end(); it++)
    {
        
        int j = 0;
        for (int i = 0; i < dataSet.columns; i++)
        {
            if (dataSet.meta[i].type == inputCol)
            {
                splitSets.sets[j].insert((*it)[i]);
                j++;
            }
            else 
            {
                splitSets.output_index = i;
                splitSets.output.insert((*it)[i]);
            }
        }
        

    }
}

    void train()
    {
        entryNode = new DecisionTreeNode(dataSet,splitSets,1);
    }

    std::string predict(DataRow row)
    {
        return DecisionTreeNode::getResult(entryNode,splitSets,row);
    }

   void confusion_matrix(DataSet* s)
{
    std::map<std::string,int> encoddings;
    int i = 0;
    std::cout<<"Clasess: ";
    for(auto it = this->splitSets.output.begin();it != this->splitSets.output.end();it++)
    {   
        std::cout<<*it<<" ";
        encoddings.insert(std::pair<std::string,int>(*it,i));
        i++;
    }
    std::cout<<"\n";
    int **mat = new int*[i];
    for(int j = 0;j<i;j++)
    {
        mat[j] = new int[i];
        for(int k = 0;k<=i;k++)
        {
            mat[j][k] = 0;
        }
    } 


    for(auto it = s->rows.begin();it!=s->rows.end();it++)
    {

        int a = encoddings.at((*it)[splitSets.output_index]);
        int b = encoddings.at(predict(*it));
        mat[a][b]++;
    }

    for(int j=0;j<i;j++)
    {
        for(int k=0;k<i;k++)
        {
            std::cout<<mat[j][k]<<" ";
        }
        std::cout<<"\n";
    }

    for(int j=0;j<i;j++)
        delete[] mat[j];
    delete[] mat;
}


    float getAccuracy(DataSet * testSet)
    {
        int num = testSet->rows.size();
        int correct = 0;
        if(num == 0)
            return 0.0f;
        
        for(auto it =testSet->rows.begin();it!=testSet->rows.end();it++)
        {
            std::string res = predict(*it);
            if(res==(*it)[splitSets.output_index])
                correct++;
        }
        return (float)(correct)/float(num);
    }
    private:
    DecisionTreeNode* entryNode = NULL;
    SplitSets splitSets;
    DataSet dataSet;
};


int main()
{  
    const ColomnMeta metas[] = 
    {
        {"status",inputCol,atrib},
        {"duration",inputCol,numericData},
        {"credit_history",inputCol,atrib},
        {"purpose",inputCol,atrib},
        {"credit",inputCol,numericData},
        {"savings",inputCol,atrib},
        {"employment",inputCol,atrib},
        {"installment_rate",inputCol,numericData},
        {"status_sex",inputCol,atrib},
        {"debtors",inputCol,atrib},
        {"resident_since",inputCol,numericData},
        {"property",inputCol,atrib},
        {"age",inputCol,numericData},
        {"installment_plans",inputCol,atrib},
        {"housing",inputCol,atrib},
        {"number_of_existing_credits",inputCol,numericData},
        {"job",inputCol,atrib},
        {"number_of_liable_people",inputCol,numericData},
        {"telephone",inputCol,atrib},
        {"local_or_foreign",inputCol,atrib},
        {"decision",outputCol,atrib}
    };
    DataSet set(21,metas);
    set.addData("dataset_credit.csv");

    DataSet** TestTrainSet = set.trainTestSplit(0.75);
    
    DecisionTree tree(*TestTrainSet[0]);
    
    tree.train();
    
    
    float accuracy = tree.getAccuracy(TestTrainSet[1])*100;
    
    std::cout<<"Model has an acurracy of "<<accuracy<<"\n";
    if(accuracy < 30)
    {
        std::cout<<"The model's performance is very poor . This may be caused by some problems in the current model\n";
    }

    else if(accuracy < 50)
    {
        std::cout<<"The model's performance is below average.Please consider using a different model or significantly improving the current one\n";
    }

    else if(accuracy < 70)
    {
        std::cout<<"The model s performance is fair, but it s not good enough. Consider tuning the model or using another model\n";
    }

    else if(accuracy < 90)
    {
        std::cout<<"The model's performance is good , but it may need more data or there may be a better suited model\n";
    }

    else {
        std::cout<<"The model's performance is excellent."<<"\n";
    }

    float accuracy2 = tree.getAccuracy(TestTrainSet[0])*100;
    if(accuracy2>accuracy)
    {
        std::cout<<"Your model presents signs of overfitting("<<accuracy<<","<<accuracy2<<")\n\n"; 
    }
    else 
    {
        std::cout<<"Your model doesn t present signs of overfitting\n\n";
    }

    std::cout<<"Confusion matrix:\n";
    tree.confusion_matrix(TestTrainSet[1]);

    

    return 0;
}