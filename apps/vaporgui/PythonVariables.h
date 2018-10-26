#ifndef PYTHOVARIABLES_H
#define PYTHOVARIABLES_H

#include <vapor/ControlExecutive.h>

#include "ui_PythonVariablesGUI.h"
#include "PythonVariablesParams.h"
#include "VaporTable.h"

#include <QDialog>
#include <QMenuBar> 
#include <QMenu>

//
// QObjects do not support nested classes, so use a namespace :\
//
namespace PythonVariables_ {
    class Fader;
    class NewItemDialog;
    class OpenAndDeleteDialog;
    
    static const string _scriptType = "Python";
}

class PythonVariables : public QDialog, Ui_PythonVariablesGUI 
{
    Q_OBJECT

public:
    PythonVariables(QWidget *parent);
    ~PythonVariables();
    void Update(bool internal = false);
    void InitControlExec(VAPoR::ControlExec* ce);
    void ShowMe();

private slots:
    void _newScript();
    void _openScript();
    void _deleteScript();
    void _importScript() {cout << "Import" << endl;}
    void _exportScript() {cout << "Export" << endl;}
    void _testScript() {cout << "Test" << endl;}
    void _applyScript();

    void _saveScript(int index);

    void _createNewVariable();
    void _deleteVariable();
    void _scriptChanged();

    void _2DInputVarChanged(int row, int col);
    void _3DInputVarChanged(int row, int col);

    void _deleteFader();

private:
    const QColor* _background;

    VAPoR::ControlExec* _controlExec;

    PythonVariables_::Fader*            _fader;
    PythonVariables_::NewItemDialog*    _newItemDialog;
    PythonVariables_::OpenAndDeleteDialog* _openAndDeleteDialog;

    VaporTable*       _2DInputVarTable;
    VaporTable*       _3DInputVarTable;
    VaporTable*       _summaryTable;
    VaporTable*       _outputVarTable;

    string _script;
    string _scriptName;
    string _dataMgrName;

    bool _justSaved;

    std::vector<string> _2DVars;
    std::vector<bool>   _2DVarsEnabled;
    std::vector<string> _3DVars;
    std::vector<bool>   _3DVarsEnabled;
    std::vector<string> _outputVars;
    std::vector<string> _outputGrids;
    std::vector<string> _inputGrids;
    std::vector<string> _otherGrids;

    void _connectWidgets();
    void _setGUIEnabled(bool enabled);
    void _makeInputTableValues(
        std::vector<string> &tableValues2D,
        std::vector<string> &tableValues3D,
        std::vector<string> &summaryValues
    ) const;
    void _makeOutputTableValues(
        std::vector<string> &outputValues
    ) const;
    std::vector<string> _makeDialogOptions(
        std::vector<string> grids
    );
    int _checkForDuplicateNames(
        std::vector<string> names,
        string name
    );
    bool _isGridSelected(
        string grid,
        std::vector<string> selectedVars,
        std::vector<bool>   varEnabled
    ) const;
    void _saveToSession();
    void _saveToFile();

    void _updateNewItemDialog();
    void _updateInputVarTable() {};
    void _updateOutputVarTable() {};
    void _updatePythonScript() {};

    void _fade(bool fadeIn);
};

namespace PythonVariables_ {

class Fader : public QObject
{
    Q_OBJECT

public:
    Fader(
        bool fadeIn,
        QLabel* label, 
        QColor background,
        QObject* parent=0
    );
    ~Fader();
    void Start();

signals:
    void faderDone();

private:
    bool _fadeIn;
    QThread* _thread;
    QLabel* _myLabel;
    QColor  _background;

private slots:
    void _fade();
};

class NewItemDialog : public QDialog
{
    Q_OBJECT

public:
    enum {
        SCRIPT = 0,
        OUTVAR = 1 
    };

    NewItemDialog(QWidget* parent=0);
    ~NewItemDialog() {};

    void Update(
        int type, 
        std::vector<string> optionNames,
        std::vector<int> categoryItems = std::vector<int>()
    );
    string GetItemName() const;
    string GetOptionName() const;

private:
    void _connectWidgets();
    void _setupGUI();
    void _adjustToType(int type);
    void _disableComboItem(int index);

    string _itemName;
    string _optionName;

    QLabel*      _itemNameLabel;
    QLineEdit*   _itemNameEdit;
    QLabel*      _optionNameLabel;
    QComboBox*   _optionNameCombo;
    QPushButton* _okButton;
    QPushButton* _cancelButton;

private slots:
    void _okClicked(); 
};

class OpenAndDeleteDialog : public QDialog
{
    Q_OBJECT

public:
    enum {
        OPEN   = 0,
        DELETE = 1
    };

    OpenAndDeleteDialog(QWidget* parent=0);
    ~OpenAndDeleteDialog() {};

    int Update(int type, VAPoR::ControlExec* controlExec);

    string GetDataMgrName() const;
    string GetScriptName() const;

private:
    void _setupGUI();

    string _dataMgrName;
    string _scriptName;

    QLabel*      _dataMgrNameLabel;
    QComboBox*   _dataMgrNameCombo;
    QLabel*      _scriptNameLabel;
    QComboBox*   _scriptNameCombo;
    QPushButton* _okButton;
    QPushButton* _cancelButton;

private slots:
    void _okClicked();
};

} // namespace PythonVariables

#endif // PYTHOVARIABLES_H