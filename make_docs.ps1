$desktop = [Environment]::GetFolderPath("Desktop")
$docPath = Join-Path $desktop "Budni_studenta_documentation.docx"
$word = New-Object -ComObject Word.Application
$word.Visible = $false
$doc = $word.Documents.Add()
$sel = $word.Selection

$sel.ParagraphFormat.Alignment = 1
$sel.Font.Size = 18
$sel.Font.Bold = $true
$sel.TypeText("Polnaya dokumentaciya")
$sel.TypeParagraph()

$sel.Font.Size = 26
$sel.TypeText("Budni studenta")
$sel.TypeParagraph()

$sel.ParagraphFormat.Alignment = 0
$sel.Font.Size = 12
$sel.Font.Bold = $false
$sel.TypeParagraph()
$sel.TypeText("Tekstovaya RPG-igra na C++")
$sel.TypeParagraph()
$sel.TypeText("Versiya: 1.0")
$sel.TypeParagraph()
$sel.TypeText("Data: iyun 2026")
$sel.TypeParagraph()
$sel.TypeText("Yazyk: Russian (lokalizovano)")
$sel.TypeParagraph()
$sel.TypeText("Platforma: Windows (MSVC), cross-platform (MinGW)")
$sel.TypeParagraph()
$sel.TypeParagraph()

# Razdel 1
$sel.Style = 2
$sel.TypeText("1. Obzor proekta")
$sel.TypeParagraph()
$sel.Style = 1
$sel.TypeText("Budni studenta - eto konsolnaya tekstovaya RPG/Visual Novel s elementami strategii i menedzhmenta resursov. Igra napisana na C++17 s ispolzovaniem tolko standartnoi biblioteki (STL).")
$sel.TypeParagraph()
$sel.TypeParagraph()
$sel.TypeText("Osnovnye mekhaniki:")
$sel.TypeParagraph()
$sel.TypeText("- Upravlenie kharakteristikami (intellekt, energiya, ustalost, golod, stress i dr.)")
$sel.TypeParagraph()
$sel.TypeText("- Sistema otnoshenii s 4 NPC (Alla, Bulat, Semen, Artem)")
$sel.TypeParagraph()
$sel.TypeText("- 5 ekzamenov s realnymi voprosami i ocenkoi")
$sel.TypeParagraph()
$sel.TypeText("- Romanticheskaya vetka s Alloi")
$sel.TypeParagraph()
$sel.TypeText("- Sluchainye sobytiya cherez menedzher sobytii")
$sel.TypeParagraph()
$sel.TypeText("- 5 debafov s sobstvennoi mekhanikoi")
$sel.TypeParagraph()
$sel.TypeText("- 10 razlichnykh koncovok")
$sel.TypeParagraph()
$sel.TypeText("- Sistema vremeni (chasy/minuty) i 6 lokacii")
$sel.TypeParagraph()
$sel.TypeText("- Sokhranenie i zagruzka cherez fstream")
$sel.TypeParagraph()

$doc.SaveAs([ref]$docPath)
Write-Host "OK: $docPath"
$word.Quit()
