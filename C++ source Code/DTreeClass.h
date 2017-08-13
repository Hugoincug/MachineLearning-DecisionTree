#pragma once
#include <iostream>
#include <fstream>  //��ȡ�ļ�����
#include <map>
#include <vector>
#include <string>
#include <opencv2/core.hpp> //��OpenCV�ĺ������������������ģ��
#include <cmath>

using std::cout; using std::cin; using std::cerr; using std::endl;
using std::string; using std::ifstream; using std::istringstream;
using std::vector; using std::map; using std::ios;

namespace machinelearning
{
	class DecisionTree
	{		
	public:
		struct TreeNode
		{   //�ڵ���Ϣ
			string Attribute;   //�˽ڵ��Ӧ������
			bool LeafNode; //�����Ҷ�ӽڵ㣬��ֵ��ӳ�������� //�����������0;
			vector<TreeNode*> children; //���ӽڵ�ĵ�ַ��
			map<string, TreeNode*> AttributeLinkChildren;  //����ָ���ӽڵ㣬Ҳ���Ƕ�Ӧ�����νṹ�е���֦

		};
		
		struct Attr    //ÿһ�е�����
		{
			int colIndex;
			string Attribute;
			int typeNum;   //����ȡֵ�ĸ���
			vector<string> AttributeValue;
			map<string, unsigned char>  typeMap; //����ȡֵ��Ӧ������ֵ;
		};
	private:
		struct MatInfo
		{
			int cols;
			int rows;

		};		
		struct entropyInfo
		{
			vector<int> labelValue;
			vector<int> labelValueNum;
		};
	public:
		cv::Mat trainDataMat;
		vector<vector<string>> predictedDataMat;
		MatInfo trainMatrixInfo;
		TreeNode *root;  //���ڵ�
		vector<Attr> vectorAttr; //�洢���еľ�����Ϣ�������洢����
		DecisionTree();
		int ReadTrainDataFile(string fileAddress);  //����Ԥ����
		TreeNode* BuildTree(cv::Mat &data, vector<Attr> &dataAttr, string AlgorithmName);  // ָ���������㷨
		vector<vector<string>> ReadPredictedDataFile(string fileAddreess);
		vector<string> Predicted(TreeNode* root, vector<vector<string>> &pData);  //����ֵΪint���ͱ�ʾ���ݵķ��ࡣ
		
	private:
		int stringDataToInt(vector<vector<string>> src, cv::Mat &dataMat, vector<Attr> &headAttrInfo);
		bool StringExistInVector(string aa, vector<string> A);
		TreeNode* AlgorithmID3(cv::Mat &data, vector<Attr> &a);
		TreeNode* AlgorithmC4_5(cv::Mat &data, vector<Attr> &a);
		TreeNode* AlgorithmCART(cv::Mat &data, vector<Attr> &a);
		//vector<float> CalculateEntropy(cv::Mat a);
		vector<float> CalculateInfGain(cv::Mat &a);
		int FindMaxInformationGain(vector<float> s);
		bool TheSameLabel(cv::Mat &a);
		cv::Mat GetNewMat(cv::Mat &a, vector<Attr> &properties, int maxIndex, string oneAttributeValue);
		int IntExistInVector(int a, vector<int> b);
		float GetDataEntropy(cv::Mat &data);
		float InformationGain(vector<int> value, map<int, entropyInfo> b, float dataEntropy, int rows);
		float Entropy(vector<float> ratio); // ������
		//�жϾ������Ƿ��и�����
		bool DataExistAttribute(cv::Mat &data, vector<Attr> &properties, int maxIndex, string oneAttributeValue);
		int IndexOFAttribute(string nodeString, vector<Attr> &vectorAttr);
		string PredictedRecursion(TreeNode* nodeAddress, vector<string> &rowData, vector<Attr> &vecAttr);
		string FindAttrString(int a, Attr b);
		string MostInMatLabel(cv::Mat &data, vector<Attr> &properties);
	};
}

