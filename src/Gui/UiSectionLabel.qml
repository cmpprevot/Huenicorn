import QtQuick
import QtQuick.Controls

// Micro uppercase section heading used above cards and groups.
Label {
    font.pixelSize: Theme.fontSmall
    font.bold: true
    font.letterSpacing: 1.4
    font.capitalization: Font.AllUppercase
    color: Theme.textFaint
}
