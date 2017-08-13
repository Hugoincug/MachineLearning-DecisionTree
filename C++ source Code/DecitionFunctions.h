#pragma once
#include "DTreeClass.h"

namespace machinelearning
{
	DecisionTree::DecisionTree()
	{
		this->root = new TreeNode;
		this->trainMatrixInfo.cols = 0;
		this->trainMatrixInfo.rows = 0;

	}
	DecisionTree::TreeNode* DecisionTree::BuildTree(cv::Mat &data, vector<Attr> &dataAttr, string Algorithm)
	{
		if (Algorithm == "ID3")
		{
			return AlgorithmID3(data, dataAttr);
		}
		else if (Algorithm == "C4.5")
		{
			return AlgorithmC4_5(data, dataAttr);
		}
		else if (Algorithm == "CART")
		{
			return AlgorithmCART(data, dataAttr);
		}
		else
		{
			cerr << "Input error, the program has been stopped!";
			exit(0);
		}
		return 0;
	}
	int DecisionTree::ReadTrainDataFile(string fileAddress)
	{
		int cols = 0, rows = 0;  //ͨ���ļ���ȡ ��ȡ�к��е���Ϣ��
								 ///cv::Mat strResult;
								 //cv::Mat result;    //���ص�����ֵ����
		vector<vector<string>> strResult;
		vector<Attr> headAttr;
		ifstream read;
		read.open(fileAddress, ios::in);

		string headline;
		getline(read, headline);
		rows++;  //ÿ��readline��Ҫ������һ��
		if (rows > 0)
		{
			string attributeName;
			char delim = ',';
			istringstream stringin(headline);
			vector<string> oneLineString;
			while (getline(stringin, attributeName, delim))
			{
				cols++;
				oneLineString.push_back(attributeName);
			}
			strResult.push_back(oneLineString);
			//cout << endl;
		}
		//��ȡ��һ����Ϣ
		string line;   //�ӵڶ����Ժ�ʼ�洢��ÿһ�е�����
					   //�����ȡ�������Ϣ		
		while (getline(read, line))
		{
			rows++;
			int realcols = 0;  //�����жϴ����������Ƿ�ͱ����е�����������ͬ����ͬ�Ŵ洢�����򲻴洢
			string attributeName;
			char delim = ',';
			istringstream stringin(line);
			vector<string> oneLineString;
			while (getline(stringin, attributeName, delim))
			{
				realcols++;
				//cout << attributeName << "  ";
				oneLineString.push_back(attributeName);
			}
			if (realcols == cols)
			{ //ֻ�кͱ���������������ͬ�Ŵ洢
				strResult.push_back(oneLineString);
			}
		}
		//cout << rows << "  " << cols;
		cv::Mat tmpMat(rows - 1, cols, CV_8UC1);
		this->trainMatrixInfo.cols = cols;
		this->trainMatrixInfo.rows = rows - 1;
		stringDataToInt(strResult, tmpMat, headAttr);
		this->trainDataMat = tmpMat;
		this->vectorAttr = headAttr;
		return 0;
	}
	int DecisionTree::stringDataToInt(vector<vector<string>> src, cv::Mat &dataMat, vector<Attr> &headAttrInfo)
	{
		int matRows = int(src.size());
		int matCols = int(src[0].size());
		for (int j = 0; j < matCols; j++)
		{
			vector<string> perColStringVector; //ÿһ�е�z�ַ�������
			map<string, unsigned char> oneColMap;
			unsigned char indexNum = 0;  //����������������͵Ĳ���ֵ��
			Attr perColAttr;
			perColAttr.Attribute = src[0][j];
			perColAttr.colIndex = j;
			for (int i = 1; i < matRows; i++)
			{
				if (StringExistInVector(src[i][j], perColStringVector))
				{
					dataMat.at<unsigned char>(i - 1, j) = oneColMap[src[i][j]];
				}
				else
				{
					dataMat.at<unsigned char>(i - 1, j) = indexNum;
					perColStringVector.push_back(src[i][j]);
					oneColMap.insert(map<string, unsigned char>::value_type(src[i][j], indexNum));
					indexNum++;
				}
			}
			perColAttr.typeMap = oneColMap;
			perColAttr.typeNum = indexNum;
			perColAttr.AttributeValue = perColStringVector;
			headAttrInfo.push_back(perColAttr);
		}
		return 0;
	}

	bool DecisionTree::StringExistInVector(string str, vector<string> strVector)
	{
		int vectorSize = int(strVector.size());
		for (int i = 0; i < vectorSize; i++)
		{
			if (strVector[i] == str)
			{
				return true;
			}
		}
		return false;
	}

	DecisionTree::TreeNode* DecisionTree::AlgorithmID3(cv::Mat &data, vector<Attr> &properties)
	{
		//�Ծ�����м��㣬����entropy,informationGain;
		//vector<float> entropy; //��Ϣ��
		vector<float> informationGain;  //��Ϣ����
		int maxIndex;  //�����Ϣ��������Ե�����
					   //entropy = CalculateEntropy(data);
		//cout << "dataSize:" << data.size() << endl;
		//cout << data << endl;
		informationGain = CalculateInfGain(data);
		maxIndex = FindMaxInformationGain(informationGain);
		if (data.cols == 2)
		{
			TreeNode* leaf = new TreeNode;
			string  label;
			label = MostInMatLabel(data, properties);
			leaf->Attribute = properties[0].Attribute;
			leaf->LeafNode = true;
			return leaf;
		}
		else if (TheSameLabel(data))
		{
			TreeNode* leaf = new TreeNode;
			int labelIndex = int(properties.size() - 1);
			int labelValue = int(data.at<unsigned char>(0, data.cols - 1));
			string label = FindAttrString(labelValue, properties[labelIndex]);
			leaf->Attribute = label;
			leaf->LeafNode = true;
			return leaf;
		}
		else
		{
			//���еݹ�
			TreeNode* branchNode = new TreeNode;
			branchNode->Attribute = properties[maxIndex].Attribute;
			branchNode->LeafNode = false;
			vector<Attr> tmpAttr = properties;   //�����µ����������������´�
			tmpAttr.erase(tmpAttr.begin() + maxIndex);  //ɾ����һ���ڵ�����ԡ�
			vector<string> attributeValue = properties[maxIndex].AttributeValue;
			//���з�֧

			for (int i = 0; i < properties[maxIndex].AttributeValue.size(); i++)
			{
				TreeNode* childNode = new TreeNode; //����һ�����ӽڵ�
				string oneAttributeValue = attributeValue[i];  //ÿһ�����Զ�Ӧ��ֵ���ַ�����
				if (DataExistAttribute(data, properties, maxIndex, oneAttributeValue))
				{
					cv::Mat subMat;
					subMat = GetNewMat(data, properties, maxIndex, oneAttributeValue);
					//����Ԥ��λ�ã�����Ԥ��֦������
					//Ԥ��֦����

					childNode = this->AlgorithmID3(subMat, tmpAttr);
					branchNode->children.push_back(childNode);
					branchNode->AttributeLinkChildren.insert(map<string, TreeNode*>::value_type(oneAttributeValue, childNode));
				}
			}
			//���ط�֧�ڵ�
			return branchNode;
		}
	}
	DecisionTree::TreeNode* DecisionTree::AlgorithmC4_5(cv::Mat &data, vector<Attr> &a)
	{
		return nullptr;
	}
	DecisionTree::TreeNode* DecisionTree::AlgorithmCART(cv::Mat &data, vector<Attr> &a)
	{
		return nullptr;
	}
	vector<float> DecisionTree::CalculateInfGain(cv::Mat &data) //����InformationGain
	{

		vector<float> result;
		float dataEntropy; //�������
		dataEntropy = GetDataEntropy(data);


		//vector<int> AttrValueNum;
		for (int i = 0; i < data.cols - 1; i++)  //ֻ�������н��м��㣬�Ա�ǩ�в����㣬��ǩ���Ѿ�������ˡ�
		{
			vector<int> attrValue;  //ÿһ�����ݹ��м������ʹ洢������
			map<int, entropyInfo> attrLinkToLabel;  //ÿ�����Ͷ�Ӧ����Ϣ����Ϣ���п��ܷ�����ͬ���п��ܷ��಻ͬ

			for (int j = 0; j < data.rows; j++)
			{
				int value = int(data.at<unsigned char>(j, i));
				if (IntExistInVector(value, attrValue) >= 0)
				{
					int index = IntExistInVector(int(data.at<unsigned char>(j, data.cols - 1)), attrLinkToLabel[value].labelValue);
					if (index == -1)
					{
						attrLinkToLabel[value].labelValue.push_back(int(data.at<unsigned char>(j, data.cols - 1)));
						attrLinkToLabel[value].labelValueNum.push_back(1);
					}
					else
					{
						attrLinkToLabel[value].labelValueNum[index]++;
					}
				}
				else
				{
					entropyInfo oneAttrEntropyInfo;
					attrValue.push_back(value);
					oneAttrEntropyInfo.labelValue.push_back(int(data.at<unsigned char>(j, data.cols - 1)));
					oneAttrEntropyInfo.labelValueNum.push_back(1);
					attrLinkToLabel.insert(map<int, entropyInfo>::value_type(value, oneAttrEntropyInfo));
				}
			}
			//������Ϣ��
			float informationGain;  //��Ϣ��
			informationGain = InformationGain(attrValue, attrLinkToLabel, dataEntropy, data.rows);
			result.push_back(informationGain);
		}
		return result;
	}
	int DecisionTree::FindMaxInformationGain(vector<float> s)
	{
		int maxIndex = 0;
		float max = 0;
		for (int i = 0; i < s.size(); i++)
		{
			if (s[i] > max)
			{
				max = s[i];
				maxIndex = i;
			}
		}
		return maxIndex;
	}
	bool DecisionTree::TheSameLabel(cv::Mat &data)
	{
		unsigned char first = data.at<unsigned char>(0, data.cols - 1);
		for (int i = 1; i < data.rows; i++)
		{
			if (data.at<unsigned char>(i, data.cols - 1) != first)
			{
				return false;
			}
		}
		return true;
	}

	cv::Mat DecisionTree::GetNewMat(cv::Mat &data, vector<Attr> &properties, int maxIndex, string oneAttributeValue)
	{
		int matCols = data.cols, matRows = data.rows;
		int dimensionOFAttr = int(properties.size());
		cv::Mat result;   //�������
		unsigned char AttrValue = properties[maxIndex].typeMap[oneAttributeValue];  //�ַ�������ֵ�ھ����ж�Ӧ����ֵ
		if (matCols != dimensionOFAttr || maxIndex >= matCols)
		{   //��һ������ά�����жϣ����ӽ�׳��
			cerr << "����ά������������ά������Ӧ";
			getchar();
			exit(0);
		}
		//����������С����Ĺ��̡�
		for (int i = 0; i < matRows; i++)
		{
			cv::Mat_<unsigned char> newRowValue;
			if (data.at<unsigned char>(i, maxIndex) == AttrValue)  //�����һ���е�Ԫ�����ַ����Զ�Ӧ����ִֵ�����������
			{
				for (int j = 0; j < matCols; j++)
				{
					if (j != maxIndex)
					{
						newRowValue.push_back(data.at<unsigned char>(i, j));
					}
				}
				//ѹ������
				newRowValue = newRowValue.t();
				//cout << newRowValue << endl;
				if (newRowValue.cols == matCols - 1)
				{  //����ѹ��
					result.push_back(newRowValue);
				}
				else
				{
					cerr << "������С����ʱ������ά���;���ά����һ��";
				}
			}
		}
		return result;
	}

	int DecisionTree::IntExistInVector(int a, vector<int> b)
	{ //����������򷵻ض�Ӧ�����������򷵻�-1
		for (int i = 0; i < b.size(); i++)
		{
			if (a == b[i])
			{
				return i;
			}
		}
		return -1;
	}

	float DecisionTree::GetDataEntropy(cv::Mat &data)
	{
		vector<float> ratio;
		vector<int> label;
		map<int, int> labelNum;
		int labelCol = data.cols - 1;
		for (int i = 0; i < data.rows; i++)
		{
			int value = int(data.at<unsigned char>(i, labelCol));
			if (IntExistInVector(value, label) >= 0)
			{
				labelNum[value]++;
			}
			else
			{
				label.push_back(value);
				labelNum.insert(map<int, int>::value_type(value, 1));
			}
		}
		for (int i = 0; i < label.size(); i++)
		{
			ratio.push_back(float(labelNum[label[i]]) / float(data.rows));

		}
		return Entropy(ratio);
	}

	float DecisionTree::InformationGain(vector<int> value, map<int, entropyInfo> b, float dataEntropy, int matRows)
	{
		float result = 0;
		for (int i = 0; i < value.size(); i++)
		{
			int D_v = 0;
			vector<float> ratio;
			for (int j = 0; j < b[value[i]].labelValueNum.size(); j++)
			{
				D_v = D_v + b[value[i]].labelValueNum[j];
			}
			for (int j = 0; j < b[value[i]].labelValueNum.size(); j++)
			{
				ratio.push_back(float(b[value[i]].labelValueNum[j]) / float(D_v));
			}
			result = result + float(float(D_v) / float(matRows)) * Entropy(ratio);

		}
		result = dataEntropy - result;
		return result;
	}

	float DecisionTree::Entropy(vector<float> ratio)
	{
		float result = 0;
		for (int i = 0; i < ratio.size(); i++)
		{
			result = result + ratio[i] * log2(ratio[i]);
		}
		return -result;
	}
	bool DecisionTree::DataExistAttribute(cv::Mat &data, vector<Attr> &properties, int maxIndex, string oneAttributeValue)
	{
		int stringValue = properties[maxIndex].typeMap[oneAttributeValue];
		for (int i = 0; i < data.rows; i++)
		{
			int dataValue = int(data.at<unsigned char>(i, maxIndex));
			if (dataValue == stringValue)
			{
				return true;
			}
		}
		return false;
	}

	vector<vector<string>> DecisionTree::ReadPredictedDataFile(string fileAddress)
	{   //��ȡ��Ԥ������
		ifstream read;
		read.open(fileAddress, ios::in);
		vector<vector<string>> predictedStringData;
		string rowString;
		while (getline(read, rowString))
		{
			string oneString;
			char dim = ',';
			istringstream stringIn(rowString);   //
			vector<string> oneRowString;
			while (getline(stringIn, oneString, dim))
			{
				oneRowString.push_back(oneString);
			}
			if (oneRowString.size() == this->trainMatrixInfo.cols - 1)
			{
				//ֻ�洢��ѵ�����ݼ�ƥ�������
				predictedStringData.push_back(oneRowString);
			}
			else
			{
				cerr << "��Ԥ�����ݺ�ѵ�����ݼ�ά����һ��" << endl;
				getchar();
				getchar();
				exit(0);
			}

		}
		this->predictedDataMat = predictedStringData;
		return predictedStringData;
	}

	vector<string> DecisionTree::Predicted(TreeNode* root, vector<vector<string>> &predictedData)
	{
		vector<string> result;
		int matCols = int(predictedData[0].size());
		int matRows = int(predictedData.size());
		for (int i = 0; i < matRows; i++)
		{
			string answer;
			answer = PredictedRecursion(root, predictedData[i], this->vectorAttr);
			result.push_back(answer);
		}
		return result;
	}
	string DecisionTree::PredictedRecursion(TreeNode* nodeAddress, vector<string> &rowData, vector<Attr> &vecAttr)
	{
		//Ԥ��ĵݹ麯��
		if (nodeAddress->LeafNode)
		{
			return nodeAddress->Attribute;
		}
		else
		{
			string nodeString = nodeAddress->Attribute;
			int attrIndex = IndexOFAttribute(nodeString, vectorAttr);
			if (attrIndex >= 0)
			{
				string attrValue = rowData[attrIndex];
				TreeNode* newRoot;
				newRoot = nodeAddress->AttributeLinkChildren[attrValue];
				return PredictedRecursion(newRoot, rowData, vecAttr);

			}
		}
		return nullptr;	
	}
	
	int DecisionTree::IndexOFAttribute(string nodeString, vector<Attr> &vectorAttr)
	{
		for (int i = 0; i < vectorAttr.size(); i++)
		{
			if (nodeString == vectorAttr[i].Attribute)
			{
				return i;
			}
		}
		return -1;
	}

	string DecisionTree::FindAttrString(int a, Attr b)
	{
		for (int i = 0; i < b.AttributeValue.size(); i++)
		{
			string tmp = b.AttributeValue[i];
			if (b.typeMap[tmp] == a)
			{
				return tmp;
			}
		}
		cout << "���ҳ�����" << endl;
		return nullptr;
	}
	string DecisionTree::MostInMatLabel(cv::Mat &data, vector<Attr> &properties)
	{
		class vecInf   //����ֻֻ��Դ˺������ڲ��࣬������ʵ�ֲ��ҵĹ��ܡ�
		{
		public:
			static struct data
			{
				int value;
				int num;
			};
			static int InStructVector(int v, vector<data> b)
			{
				for (int i = 0; i < b.size(); i++)
				{
					if (v == b[i].value)
					{
						return i;
					}
				}
				return -1;
			}
		};
		int rows = data.rows;
		vector<vecInf::data> intVec;
		for (int i = 0; i < rows; i++)
		{
			int labelValue = int(data.at<unsigned char>(i, data.cols - 1));
			int indexOFVec = vecInf::InStructVector(labelValue, intVec);
			if (indexOFVec >= 0)
			{
				intVec[indexOFVec].num++;
			}
			else
			{
				vecInf::data tmp;
				tmp.value = labelValue;
				tmp.num = 1;
				intVec.push_back(tmp);
			}
		 }
		int maxValue = 0, maxNum = 0;
		for (int i = 0; i < intVec.size(); i++)
		{
			if (intVec[i].num >= maxNum)
			{
				maxValue = intVec[i].value;
			}
		}
		string result = FindAttrString(maxValue, properties[properties.size() - 1]);
		return result;
	}
}

