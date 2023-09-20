const vscode = require('vscode');

const HELLO = 'fx.hello'

function hello() { vscode.window.showInformationMessage(HELLO); }

async function activate(context) {
    console.log(activate, context);
    vscode.window.showInformationMessage(activate);
    let disposable = vscode.commands.registerCommand(HELLO, hello);
    context.subscriptions.push(disposable);
}

function deactivate() {
    console.log(deactivate);
    vscode.window.showInformationMessage(deactivate);
}

module.exports = {
    activate,
    deactivate
}
