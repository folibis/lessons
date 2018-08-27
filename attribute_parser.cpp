#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

typedef struct Attribute {
    string name;
    string value;
}Attribute;

typedef struct Node {
    string name = "";
    vector<Node *> children;
    vector<Attribute> attributes;
    Node *parent = nullptr;
}Node;

enum class State {
    None,
    ParsingTag,
    InsideTag,
    ParsingAttribute,
    InsideAttribute,
    ParsingAttribute2,
    ParsingAttributeValue,
    ParsingCloseTag,
    NoneQuery,
    ParsingQueryTag,
    ParsingQueryAttribute,
};

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)
class Parser;
typedef struct Action {
    char ch ;
    State state;
    void (Parser::*func)(char);
}Action;
#pragma pack(pop)

#define ACTIONS_COUNT 20

class Parser {
public:
    void parse(const string &str);
    Attribute *query(const string &query);

private:
    void checkState(char ch);
    void startTagAction(char ch);
    void parsingTagAction(char ch);
    void enterTagAction(char ch);
    void parsingAttributeAction(char ch);
    void enterAttributeAction(char ch);
    void equalAttributeAction(char ch);
    void startAttributeValueAction(char ch);
    void parsingAttributeValueAction(char ch);
    void enterAttributeValueAction(char ch);
    void endTagAction(char ch);
    void startCloseTagAction(char ch);
    void parsingCloseTagAction(char ch);
    void enterCloseTagAction(char ch);
    void parsingQueryTagAction(char ch);
    void parsingQueryNextTagAction(char ch);
    void parsingQueryAttributeAction(char ch);

protected:
    Node root;
    Node *current = &root;
    State currentState = State::None;
    string temp;
    string temp2;
    bool wrongQuery = false;
    Action actions[ACTIONS_COUNT] = {
        { '<', State::None, &Parser::startTagAction },
        { '/', State::ParsingTag, &Parser::startCloseTagAction },
        { ' ', State::ParsingTag, &Parser::enterTagAction },
        { '>', State::ParsingTag, &Parser::enterTagAction },
        { '*', State::ParsingTag, &Parser::parsingTagAction },
        { '>', State::InsideTag, &Parser::endTagAction },
        { '*', State::InsideTag, &Parser::parsingAttributeAction },
        { ' ', State::ParsingAttribute, &Parser::enterAttributeAction },
        { '*', State::ParsingAttribute, &Parser::parsingAttributeAction },
        { '=', State::InsideAttribute, &Parser::equalAttributeAction },
        { '"', State::ParsingAttribute2, &Parser::startAttributeValueAction },
        { '"', State::ParsingAttributeValue, &Parser::enterAttributeValueAction },
        { '*', State::ParsingAttributeValue, &Parser::parsingAttributeValueAction },
        { '>', State::ParsingCloseTag, &Parser::enterCloseTagAction },
        { '*', State::ParsingCloseTag, &Parser::parsingCloseTagAction },

        { '*', State::NoneQuery, &Parser::parsingQueryTagAction },
        { '.', State::ParsingQueryTag, &Parser::parsingQueryNextTagAction },
        { '~', State::ParsingQueryTag, &Parser::parsingQueryNextTagAction },
        { '*', State::ParsingQueryTag, &Parser::parsingQueryTagAction },
        { '*', State::ParsingQueryAttribute, &Parser::parsingQueryAttributeAction },
    };
};

void Parser::parse(const string &str)
{
    for(unsigned int i = 0;i < str.length();i ++)
    {
        char ch = str[i];
        checkState(ch);
    }
}

void Parser::checkState(char ch)
{
    for(unsigned int i = 0;i < ACTIONS_COUNT;i ++)
    {
        const Action &action = actions[i];
        if( (action.ch == ch || action.ch == '*') && action.state == currentState)
        {
            void (Parser::*f)(char) = action.func;
            (this->*f)(ch);
            return;
        }
    }
}

void Parser::startTagAction(char)
{
    temp = "";
    currentState = State::ParsingTag;
}

void Parser::startCloseTagAction(char)
{
    temp = "";
    currentState = State::ParsingCloseTag;
}

void Parser::parsingTagAction(char ch)
{
    temp += ch;
}

void Parser::enterTagAction(char ch)
{
    Node *node = new Node;
    node->name = temp;
    temp = "";
    node->parent = current;
    current->children.push_back(node);
    current = node;
    if(ch == '>')
        currentState = State::None;
    else
        currentState = State::InsideTag;
}

void Parser::parsingAttributeAction(char ch)
{
    if(ch == ' ')
        return;
    temp += ch;
    currentState = State::ParsingAttribute;
}

void Parser::enterAttributeAction(char)
{
    currentState = State::InsideAttribute;
}

void Parser::equalAttributeAction(char)
{
    currentState = State::ParsingAttribute2;
}

void Parser::startAttributeValueAction(char)
{
    temp2 = "";
    currentState = State::ParsingAttributeValue;
}

void Parser::parsingAttributeValueAction(char ch)
{
    temp2 += ch;
}

void Parser::enterAttributeValueAction(char)
{
    Attribute attr;
    attr.name = temp;
    attr.value = temp2;
    current->attributes.push_back(attr);
    temp = "";
    temp2 = "";
    currentState = State::InsideTag;
}

void Parser::endTagAction(char)
{
    currentState = State::None;
}

void Parser::parsingCloseTagAction(char ch)
{
    if(ch == ' ')
        return;
    temp += ch;
}

void Parser::enterCloseTagAction(char)
{
    if(temp == current->name)
    {
        current = current->parent;
    }
    temp = "";
    currentState = State::None;
}

Attribute *Parser::query(const string &query)
{
    temp = "";
    temp2 = "";
    current = &root;
    wrongQuery = false;
    currentState = State::NoneQuery;
    for(unsigned int i = 0;i < query.length();i ++)
    {
        char ch = query[i];
        checkState(ch);
        if(wrongQuery == true)
            return nullptr;
    }

    for(unsigned int i = 0;i < current->attributes.size();i ++)
    {
        if(current->attributes[i].name == temp)
        {
            return &(current->attributes[i]);
        }
    }
    return nullptr;
}

void Parser::parsingQueryTagAction(char ch)
{
    temp += ch;
    currentState = State::ParsingQueryTag;
}

void Parser::parsingQueryNextTagAction(char ch)
{
    Node *node = nullptr;
    for(unsigned int i = 0;i < current->children.size();i ++)
    {
        node = current->children[i];
        if(temp == node->name)
        {
            current = node;
            temp = "";
            if(ch == '~')
                currentState = State::ParsingQueryAttribute;
            return;
        }
    }
    wrongQuery = true;
}

void Parser::parsingQueryAttributeAction(char ch)
{
    temp += ch;
}

int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */

    /*
    string lines = "\
<a>\
<b name = \"tag_one\">\
<c name = \"tag_two\" value = \"val_907\">\
</c>\
</b>\
</a>";
    vector<string> queries;
    queries.push_back("a.b~name");
    queries.push_back("a.b.c~value");
    queries.push_back("a.b.c~src");
    queries.push_back("a.b.c.d~name");
    */

    string lines = "";
    string temp;
    vector<string> queries;
    int linesNum, queriesNum;
    cin >> linesNum >> queriesNum;
    getline(cin, temp);
    for(int i = 0;i < linesNum;i ++)
    {
        getline(cin, temp);
        lines += temp;
    }
    for(int i = 0;i < queriesNum;i ++)
    {
        getline(cin, temp);
        queries.push_back(temp);
    }

    Parser parser;
    parser.parse(lines);
    for(unsigned int i = 0;i < queries.size();i ++)
    {
        Attribute *attr = parser.query(queries[i]);
        if(attr == nullptr)
            cout << "Not Found!" << endl;
        else
            cout << attr->value << endl;
    }

    return 0;
}
