#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <list>
#include <string>
#include <sstream>
#include <iostream>
#include <utility>
#include <random>

enum columnType { inputCol, outputCol };
enum columnDataType { numericData, atrib };

static inline void trim(std::string &s)
{
    if (isspace(s[s.length() - 1]))
        s.pop_back();
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

class NumericRow
{
public:
    NumericRow(int size, float *values)
    {
        this->size = size;
        if (values == NULL || size <= 0)
        {
            this->values = NULL;
            this->size = 0;
        }
        else
        {
            this->values = new float[size];
            for (int i = 0; i < size; i++)
            {
                this->values[i] = values[i];
            }
        };
    }

    NumericRow(const NumericRow &row)
    {
        this->size = row.size;
        if (row.values == NULL || row.size <= 0)
        {
            this->values = NULL;
            this->size = 0;
        }
        else
        {
            this->values = new float[size];
            for (int i = 0; i < size; i++)
            {
                this->values[i] = row.values[i];
            }
        };
    }

    float operator[](int index) const
    {
        if (index >= size)
        {
            throw DataException("DataRow Index out of bounds");
        }
        else
            return values[index];
    }
    std::string toString() const
    {
        std::string res = "";
        if (size > 0)
        {
            res += std::to_string(values[0]);
            for (int i = 1; i < size; i++)
            {
                res += "," + std::to_string(values[i]);
            }
        }
        return res;
    }

    ~NumericRow()
    {
        if (this->values != NULL)
        {
            delete[] values;
            values = NULL;
        }
    }

private:
    int size;
    float *values;
};

struct ColomnMeta
{   
    std::string label;
    columnType type;
    columnDataType dataType;
    float begin;
    float end;
    int size;
    std::string *listOfValues;
};

class NumericDataSet
{
public:
    NumericDataSet(int columns, ColomnMeta *meta)
    {
        this->columns = columns;
        if (columns <= 0 || meta == NULL)
        {
            this->meta = NULL;
            this->columns = 0;
        }
        else
        {
            this->meta = new ColomnMeta[columns];
            for (int i = 0; i < columns; i++)
            {
                this->meta[i] = meta[i];
            }
        }
    }

    NumericDataSet(const NumericDataSet &set)
    {
        this->columns = set.columns;
        if (set.columns <= 0 || set.meta == NULL)
        {
            std::cout<<"bad";
            this->meta = NULL;
            this->columns = 0;
        }
        else
        {
            this->meta = new ColomnMeta[columns];
            for (int i = 0; i < columns; i++)
            {
                this->meta[i] = set.meta[i];
            }
        }

        for (auto it = set.rows.begin(); it != set.rows.end(); it++)
        {
            this->rows.push_front(*it);
        }
    }

    NumericDataSet** TrainTestSplit(float train_sample_size_percentage)
    {
        float test_semple = 1.0f-train_sample_size_percentage;
        NumericDataSet** newSets = new NumericDataSet*[2];
        newSets[0] = new NumericDataSet(this->columns,this->meta);
        newSets[1] = new NumericDataSet(this->columns,this->meta);
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

    void addData(std::string filename)
    {
        std::ifstream f(filename);
        while (!f.eof())
        {
            std::string unformatted;
            std::getline(f, unformatted);
            std::istringstream tokener(unformatted);
            float *values = new float[columns];
            for (int i = 0; i < columns; i++)
            {
                bool choice = this->meta[i].dataType == numericData;
                std::string formatted;
                std::getline(tokener, formatted, ',');
                trim(formatted);
                if (choice)
                {
                    try
                    {
                        values[i] = std::stof(formatted);
                    }
                    catch (std::exception e)
                    {
                        values[i] = 0;
                    }
                }
                else
                {
                    int j = 0;
                    bool found = 0;
                    while (j < this->meta[i].size && !found)
                    {
                        if (formatted == meta[i].listOfValues[j])
                            found = 1;
                        else
                            j++;
                    }
                    if (j >= this->meta[i].size)
                    {
                        std::cout << "Corrupted data";
                        values[i] = 0;
                    }
                    else
                        values[i] = j;
                }
            }
            rows.push_front(NumericRow(columns, values));
            delete[] values;
        }
        f.close();
    };

    void show() const
    {
        for (int i = 0; i < columns; i++)
        {
            std::cout << meta[i].label << ",";
        }
        for (auto it = rows.begin(); it != rows.end(); it++)
        {
            std::cout << it->toString() << "\n";
        }
    }

    float getVariance(int outputCol) const
    {
        float variance = 0;
        float avg = 0;
        for (auto it = this->rows.begin(); it != this->rows.end(); it++)
        {
            avg += (*it)[outputCol];
        }
        avg /= rows.size();

        for (auto it = this->rows.begin(); it != this->rows.end(); it++)
        {
            float x = (*it)[outputCol] - avg;
            variance += x * x;
        }

        return variance / rows.size();
    }

    NumericDataSet **split(float med, int col_index) const
    {
        NumericDataSet **set = new NumericDataSet *[2];
        set[0] = new NumericDataSet(this->columns, this->meta);
        set[1] = new NumericDataSet(this->columns, this->meta);

        for (auto it = this->rows.begin(); it != this->rows.end(); it++)
        {
            if ((*it)[col_index] <= med)
            {
                set[0]->rows.push_front(NumericRow((*it)));
            }
            else
            {
                set[1]->rows.push_front(NumericRow((*it)));
            }
        }
        return set;
    };

    ~NumericDataSet()
    {
        if (meta != NULL)
        {
            delete[] meta;
            meta = NULL;
        }
    }

private:
    friend class RegressionTree;
    friend class RegressionNode;
    ColomnMeta *meta;
    std::list<NumericRow> rows;
    int columns;
};

class NumericSplit
{
public:
    ~NumericSplit()
    {
        if (intervals)
        {
            delete[] intervals;
            intervals = NULL;
        }
    }
    NumericSplit **split(int index, float med) const
    {
        NumericSplit **splits = new NumericSplit *[2];

        splits[0] = new NumericSplit();
        splits[1] = new NumericSplit();

        splits[0]->output = {output.first, output.second};
        splits[1]->output = {output.first, output.second};

        splits[0]->size = size;
        splits[1]->size = size;

        splits[0]->output_index = output_index;
        splits[1]->output_index = output_index;

        splits[0]->intervals = new std::pair<float, float>[size];
        splits[1]->intervals = new std::pair<float, float>[size];

        for (int i = 0; i < size; i++)
        {
            if (i != index)
            {
                splits[0]->intervals[i].first = this->intervals[i].first;
                splits[0]->intervals[i].second = this->intervals[i].second;
                splits[1]->intervals[i].first = this->intervals[i].first;
                splits[1]->intervals[i].second = this->intervals[i].second;
            }
            else
            {
                splits[0]->intervals[i].first = this->intervals[i].first;
                splits[0]->intervals[i].second = med;
                splits[1]->intervals[i].first = med;
                splits[1]->intervals[i].second = this->intervals[i].second;
            }
        }

        return splits;
    }
    std::pair<float, float> *intervals = NULL;
    int size = 0;
    std::pair<float, float> output;
    int output_index;
};

class RegressionNode
{
    public:    
    float getReduction(const NumericDataSet& set,int col_index,int output_index,float med,float& varianceL,float&varianceR)
    {
        float avgL=0;
        float avgR=0;
        float weightL=0;
        float weightR=0;
        varianceL = 0;
        varianceR = 0;

        for(auto it=set.rows.begin();it!=set.rows.end();it++)
        {
            if((*it)[col_index]<=med)
            {
                avgL+=(*it)[output_index];
                weightL++;
            }
            else 
            {
                avgR+=(*it)[output_index];
                weightR++;            
            }
        }

        if(weightL>0)
            avgL/=weightL;
        else 
            return -MAXFLOAT;
        
        if(weightR>0)
            avgR/=weightR;
        else 
           return -MAXFLOAT;

        for(auto it=set.rows.begin();it!=set.rows.end();it++)
        {
            if((*it)[col_index]<=med)
            {
            float x = (*it)[output_index] - avgL;
            varianceL += x*x;
            }
            else
            {
            float x = (*it)[output_index] - avgR;
            varianceR += x*x;
            }
        }

        varianceL/=weightL;
        varianceR/=weightR;

        return this->variance - (weightL/(set.rows.size()))*varianceL
                              - (weightR/(set.rows.size()))*varianceR;
    }    

    static void createSample(float* sample,int size,float begin,float end)
    {
        std::random_device rand_dev;
        std::mt19937 generator(rand_dev());
        std::uniform_real_distribution<float> distribution(begin,end);
        for(int i=0;i<size;i++)
        {
            sample[i] = distribution(generator);
        }
    }
    
    int stop(const NumericDataSet& set,const NumericSplit& split,float minRowsPerLeaf)
    {
        if(variance==0)
            return 1;

        if(set.rows.size()<=minRowsPerLeaf)
            return 1;
        
        if(set.rows.empty())
            return 2;
        
        float dx = 0.005;
        int stop = 1;
        for(int i=0;i<split.size;i++)
        {
            if(std::abs(split.intervals[i].first-split.intervals[i].second)>dx)
            {
                stop = 0;
            }
        }


        return stop;

    }

    RegressionNode(const NumericDataSet& set,const NumericSplit& split,float variance,int sampleSize,int minRowsPerLeaf)
    {

        this->variance = variance;
        float savedIndex=-1;
        float savedValue;
        float reduction = 0;
        float varianceL;
        float varianceR;

      
        int check = stop(set,split,minRowsPerLeaf);

        if(check==1)
            {
                this->value = 0;
                this->left = NULL;
                this->right = NULL;
                for(auto it=set.rows.begin();it!=set.rows.end();it++)
                {
                    
                    this->value += (*it)[split.output_index];
                }
                this->value/=set.rows.size();
                return;
            }
        if(check==2)
        {
            this->value = 0;
            this->left = NULL;
            this->right = NULL;
            return;
        }
        
        for(int i=0;i<set.columns;i++)
        {
            if(set.meta[i].type!=outputCol)
            {
                float *sample = new float[sampleSize];
                createSample(sample,sampleSize,split.intervals[i].first,split.intervals[i].second);
                
                for(int med=0;med<sampleSize;med++)
                {
                    float newVarianceL;
                    float newVarianceR;
                    float newReduction = getReduction(set,i,split.output_index,sample[med], newVarianceL, newVarianceR);

                    if(newReduction>reduction)
                    {
                        varianceL = newVarianceL;
                        varianceR = newVarianceR;
                        reduction = newReduction;
                        savedValue = sample[med];
                        savedIndex = i;
                    }
                }
                delete[] sample; 
            }
        }
        
        this->median = savedValue;
        this->culomn_index = savedIndex;

        if(this->culomn_index==-1)
        {
                this->value = 0;
                this->left = NULL;
                this->right = NULL;
                for(auto it=set.rows.begin();it!=set.rows.end();it++)
                {
                    
                    this->value +=(*it)[split.output_index];
                }
                this->value/=set.rows.size();
            return;
        }

        NumericDataSet** newSets = set.split(this->median,this->culomn_index);
        NumericSplit** newSplits = split.split(this->culomn_index,this->median);

  
        if(newSets[0]->rows.size()==0 || newSets[1]->rows.size()==0)
        {
                this->value = 0;
                this->left = NULL;
                this->right = NULL;
                for(auto it=set.rows.begin();it!=set.rows.end();it++)
                {
                    
                    this->value +=(*it)[split.output_index];
                }
                this->value/=set.rows.size();
        }

        else
        {
            this->left = new RegressionNode(*newSets[0],*newSplits[0],varianceL,sampleSize,minRowsPerLeaf);
            this->right = new RegressionNode(*newSets[1],*newSplits[1],varianceR,sampleSize,minRowsPerLeaf);

        }

            delete newSets[0];
            delete newSets[1];
            delete newSplits[0]; 
            delete newSplits[1]; 
            delete[] newSets;
            delete[] newSplits; 

    }
    ~RegressionNode()
    {
        if(left)
        {
            delete left;
            left = NULL; 
        }
        if(right)
        {
            delete right;
            right = NULL;
        }
    }

    
    private:
    friend class RegressionTree;
    float value;
    float median;
    float culomn_index;
    float variance;
    RegressionNode *left;
    RegressionNode *right;

};

class RegressionTree
{
    public:
    RegressionTree(const NumericDataSet& set,int sampleSize,int minRowsPerLeaf):
    set(set),sampleSize(sampleSize),minRowsPerLeaf(minRowsPerLeaf)
    {
        this->split.size = set.columns;
        this->split.intervals = new std::pair<float,float>[split.size];
        
        int j=0;
        
        for(int i=0;i<set.columns;i++)
        {
            
            if(set.meta[i].type==inputCol)
            {
                
                this->split.intervals[j].first=set.meta[i].begin;
                this->split.intervals[j].second=set.meta[i].end;
                j++;
            }
            else 
            {
                this->split.output.first = set.meta[i].begin;
                this->split.output.second=set.meta[i].end;
                this->split.output_index = i;
                
            }
           
        }
        
    }

    void train()
    {
        this->entry = new RegressionNode(set,split,set.getVariance(split.output_index),sampleSize,minRowsPerLeaf);
    }


    ~RegressionTree()
    {

    }
    
    float predict(NumericRow row)
    {
        RegressionNode* temp = entry;
        while(temp->left && temp->right!=0)
        {
            if(row[temp->culomn_index]<=temp->median)
                temp = temp->left;
            else
                temp = temp->right;
        }
        return temp->value;
    }

    float getAccuracy(NumericDataSet* train,bool aprox=0,float error = 0)
    {
        int num = 0;
        for(auto it=train->rows.begin();it!=train->rows.end();it++)
        {
            
            float predicted = predict(*it);
            if(aprox==1)
            {
                predicted  = std::round(predicted );
                if(predicted ==(*it)[split.output_index])
                    num++;
            }
            else 
            {
                if(std::abs((*it)[split.output_index]-predicted )<=error)
                    num++;
            }
        }
        return float(num)/(train->rows.size());
    }

        void evaluate(NumericDataSet** sets)
    {
        float accuracy1 = this->getAccuracy(sets[1])*100;
        float accuracy2 = this->getAccuracy(sets[0])*100;
        std::cout<<"Accuracy: "<<accuracy1<<"\n";
        if(accuracy1<accuracy2)
        {
            std::cout<<"The model presents signs of overfitting";
        }
        else 
        {
            std::cout<<"The model doesnt present signs of overfitting";
        }
        std::cout<<"("<<accuracy1<<","<<accuracy2<<")\n";

        int size = set.meta[split.output_index].size;
        float** confusion_matrix = new float*[size];
        for(int i=0;i<size;i++)
        {
            confusion_matrix[i] = new float[size];
            for(int j=0;j<size;j++)
                confusion_matrix[i][j] = 0;
        }

        for(auto it=sets[1]->rows.begin();it!=sets[1]->rows.end();it++)
        {
            int trueVal = std::round((*it)[split.output_index]);
            int predicted = std::round(predict(*it));
            //int predicted = int(predict(*it));
            confusion_matrix[trueVal][predicted]++;
        }

        for(int i=0;i<size;i++)
        {
            for(int j=0;j<size;j++)
            {
                std::cout<<confusion_matrix[i][j]<<" ";
            }
            std::cout<<"\n";
        }
    }    

    private:
    int sampleSize;
    int minRowsPerLeaf;
    NumericDataSet set;
    NumericSplit split;
    RegressionNode* entry=NULL;
};


int main()
{
    std::string list1[] = {"vhigh","high","med","low"};
    std::string list2[] = {"vhigh","high","med","low"};
    std::string list3[] = {"2","3","4","5more"};
    std::string list4[] = {"2","4","more"};
    std::string list5[] = {"small","med","big"};
    std::string list6[] = {"low","med","high"};
    std::string outputList[] = {"unacc","acc","good","vgood"};

    ColomnMeta metas[] =
    
    {
        {"price",inputCol,atrib,0,3,4,list1},
        {"maintenance",inputCol,atrib,0,3,4,list2},
        {"doors",inputCol,atrib,0,3,4,list3},
        {"people",inputCol,atrib,0,2,3,list4},
        {"luggage",inputCol,atrib,0,2,3,list5},
        {"safety",inputCol,atrib,0,2,3,list6},
        {"decision",outputCol,atrib,0,3,4,outputList}
    };

    NumericDataSet dataSet(7,metas);
    dataSet.addData("car_evaluation.csv");
    NumericDataSet** sets = dataSet.TrainTestSplit(0.75);
    RegressionTree* tree = new RegressionTree(*sets[0],6,5);
    tree->train();
    tree->evaluate(sets);
    return 0;
}