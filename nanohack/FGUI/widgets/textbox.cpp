//
// FGUI - feature rich graphical user interface
//

// library includes
#include "textbox.hpp"
#include "container.hpp"
#include <cwctype>

namespace FGUI
{

    CTextBox::CTextBox()
    {
        m_strTitle = "";
        m_dmSize = { 150, 20 };
        m_bIsGettingKey = false;
        m_strCustomText = "";
        m_uiLength = 24;
        m_anyFont = 0;
        m_uiInputPos = 0;
        m_strTooltip = "";
        m_bIsTextSelected = false;
        m_nStyle = static_cast<int>(TEXTBOX_STYLE::NORMAL);
        m_nType = static_cast<int>(WIDGET_TYPE::TEXTBOX);
        m_nFlags = static_cast<int>(WIDGET_FLAG::DRAWABLE) | static_cast<int>(WIDGET_FLAG::CLICKABLE) | static_cast<int>(WIDGET_FLAG::SAVABLE);
    }

    void CTextBox::SetText(std::string text)
    {
        m_strCustomText = text;
    }

    std::string CTextBox::GetText()
    {
        return m_strCustomText;
    }

    void CTextBox::SetLength(unsigned int length)
    {
        m_uiLength = length;
    }

    void CTextBox::SetStyle(FGUI::TEXTBOX_STYLE style)
    {
        m_nStyle = static_cast<int>(style);
    }

    void CTextBox::Geometry()
    {
        FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };

        FGUI::DIMENSION dmTitleTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strTitle);

        FGUI::DIMENSION dmTypedTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strCustomText);

        // textbox body
        if (FGUI::INPUT.IsCursorInArea(arWidgetRegion) || m_bIsGettingKey)
        {
            FGUI::RENDER.Outline(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 0,0,0 });
            FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 1), (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 46,46,46 });
        }
        else
        {
            FGUI::RENDER.Outline(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 26,26,26 });
            FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 1), (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 46,46,46 });
        }

        // textbox label
        FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 10), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 255,255,255 }, m_strTitle + ":");

        // textbox typed text
        if (m_nStyle == static_cast<int>(FGUI::TEXTBOX_STYLE::NORMAL))
        {
            FGUI::RENDER.Text(arWidgetRegion.m_iLeft + (dmTitleTextSize.m_iWidth + 20), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 255,255,255 }, m_strCustomText);
        }
        else if (m_nStyle == static_cast<int>(FGUI::TEXTBOX_STYLE::UPPERCASE))
        {
            std::transform(m_strCustomText.begin(), m_strCustomText.end(), m_strCustomText.begin(), ::toupper);

            FGUI::RENDER.Text(arWidgetRegion.m_iLeft + (dmTitleTextSize.m_iWidth + 20), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 255,255,255 }, m_strCustomText);
        }
        else if (m_nStyle == static_cast<int>(FGUI::TEXTBOX_STYLE::PASSWORD))
        {
            FGUI::RENDER.Text(arWidgetRegion.m_iLeft + (dmTitleTextSize.m_iWidth + 20), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 255,255,255 }, std::string(m_strCustomText.length(), '*'));
        }

        if (m_bIsGettingKey && !m_bIsTextSelected) {

            // temporary text that we will use to calculate caret position
            std::string strTemporaryText = m_strCustomText;
            strTemporaryText.erase(m_uiInputPos, strTemporaryText.size());

            // temporary text size
            FGUI::DIMENSION strTemporaryTextSize = FGUI::RENDER.GetTextSize(m_anyFont, strTemporaryText);

            // draw caret
            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + (dmTitleTextSize.m_iWidth + 20) + (strTemporaryTextSize.m_iWidth + 1), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), 1, dmTitleTextSize.m_iHeight, { 1, 1, 1 });
        }

        // selected text rectangle
        if (m_bIsTextSelected && m_strCustomText.length() > 0)
        {
            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + (dmTitleTextSize.m_iWidth + 20), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), dmTypedTextSize.m_iWidth, dmTypedTextSize.m_iHeight, { 38, 165, 255, 100 });
        }
    }

    void CTextBox::Update()
    {
        if (m_bIsGettingKey)
        {
            static bool bSpecialKeyHeld = false;

            for (std::size_t key = 0; key < 256; key++)
            {
                if (!FGUI::INPUT.IsKeyPressed(key))
                {
                    continue;
                }

                // this will be used to stop the textbox from receiving input when a special key is held
                if (FGUI::INPUT.IsKeyHeld(KEY_LCONTROL) || FGUI::INPUT.IsKeyHeld(KEY_RCONTROL))
                {
                    bSpecialKeyHeld = true;
                }
                else
                {
                    bSpecialKeyHeld = false;
                }

                // key input
                std::string strKeyInput;

                // change status to currently pressed key
                switch (FGUI::INPUT.GetInputType())
                {
                case static_cast<int>(INPUT_TYPE::WIN_32):
                {
                    strKeyInput = m_ksStrings.m_strVirtualKeyCodes[key].data();
                    break;
                }
                case static_cast<int>(INPUT_TYPE::INPUT_SYSTEM):
                {
                    strKeyInput = m_ksStrings.m_strInputSystem[key].data();
                    break;
                }
                default:
                {
                    std::throw_with_nested(std::runtime_error(""));
                    break;
                }
                }

                // clear text if the user types something while the text is selected
                if (m_bIsTextSelected)
                {
                    m_bIsTextSelected = false;
                    m_strCustomText.clear();
                    m_uiInputPos = 0;
                }

                // if the user press ESC or ENTER, stop the textbox from receiving input
                if (key == KEY_ESCAPE || key == KEY_ENTER)
                {
                    // block textbox
                    m_bIsGettingKey = false;
                    bSpecialKeyHeld = false;
                }

                if (key == KEY_SPACE)
                {
                    m_strCustomText.insert(m_uiInputPos, " ");
                }

                // handle upper case keys
                if (FGUI::INPUT.IsKeyHeld(KEY_LSHIFT) || FGUI::INPUT.IsKeyHeld(KEY_RSHIFT))
                {
                    std::transform(strKeyInput.begin(), strKeyInput.end(), strKeyInput.begin(), ::toupper);
                }

                // insert text
                if (strKeyInput.length() == 1 && (m_strCustomText.length() < m_uiLength) && !bSpecialKeyHeld)
                {
                    m_strCustomText.insert(m_uiInputPos, strKeyInput);
                    m_uiInputPos++;
                }

                if (key == KEY_SPACE)
                {
                    m_strCustomText.insert(m_uiInputPos, " ");
                    m_uiInputPos++;
                }

                if (m_strCustomText.length() > 0)
                {
                    if (key == KEY_BACKSPACE && (m_uiInputPos > 0))
                    {
                        m_uiInputPos--;
                        m_strCustomText.erase(m_uiInputPos, 1);
                    }
                    else if ((FGUI::INPUT.IsKeyHeld(KEY_LCONTROL) && key == KEY_A))
                    {
                        m_bIsTextSelected = true;

                        if (m_bIsTextSelected)
                        {
                            if (key == KEY_BACKSPACE && (m_uiInputPos > 0))
                            {
                                m_bIsTextSelected = false;
                                bSpecialKeyHeld = false;
                                m_strCustomText.clear();
                                m_uiInputPos = 0;
                            }
                        }
                    }
                    else if (key == KEY_DELETE && (m_uiInputPos > 0))
                    {
                        m_strCustomText.clear();
                        m_uiInputPos = 0;
                    }

                    // change the current input position to the previous word
                    if (FGUI::INPUT.IsKeyHeld(KEY_LCONTROL) && key == KEY_LEFT && m_uiInputPos > 0)
                    {
                        // search left until we hit a nonspace character
                        while (--m_uiInputPos > 0)
                        {
                            if (!std::iswspace(m_strCustomText[m_uiInputPos]))
                            {
                                break;
                            }
                        }

                        // search left until we hit a whitespace character
                        while (--m_uiInputPos > 0)
                        {
                            if (std::iswspace(m_strCustomText[m_uiInputPos]))
                            {
                                break;
                            }
                        }
                    }
                    else if (key == KEY_LEFT && m_uiInputPos > 0) // change the current input position to the next char
                    {
                        m_uiInputPos--;
                    }

                    // change current input position to the next word
                    if (FGUI::INPUT.IsKeyHeld(KEY_LCONTROL) && key == KEY_RIGHT && (m_uiInputPos < m_strCustomText.length()))
                    {
                        // search right until we hit a whitespace character
                        while (++m_uiInputPos < m_strCustomText.length())
                        {
                            if (std::iswspace(m_strCustomText[m_uiInputPos]))
                            {
                                break;
                            }
                        }

                        // search right until we hit a nonspace character
                        while (++m_uiInputPos < m_strCustomText.length())
                        {
                            if (!std::iswspace(m_strCustomText[m_uiInputPos]))
                            {
                                break;
                            }
                        }

                        if (m_uiInputPos >= m_strCustomText.length())
                        {
                            m_uiInputPos = (m_strCustomText.length() - 1);
                        }
                    }
                    else if (key == KEY_RIGHT && (m_uiInputPos < m_strCustomText.length())) // change the current input position to the next char
                    {
                        m_uiInputPos++;
                    }
                }
            }
        }

        // stop receiving input if another widget is being focused
        if (std::reinterpret_pointer_cast<FGUI::CContainer>(GetParentWidget())->GetFocusedWidget())
        {
            m_bIsGettingKey = false;
        }
    }

    void CTextBox::Input()
    {
        FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };

        if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
        {
            m_bIsGettingKey = !m_bIsGettingKey;
        }
    }

    void CTextBox::Save(nlohmann::json& module)
    {
        // remove spaces from widget name
        std::string strFormatedWidgetName = GetTitle();
        std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

        module[strFormatedWidgetName] = m_strCustomText;
    }

    void CTextBox::Load(nlohmann::json& module)
    {
        // remove spaces from widget name
        std::string strFormatedWidgetName = GetTitle();
        std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

        // change widget text to the one stored on file
        if (module.contains(strFormatedWidgetName))
        {
            m_strCustomText = module[strFormatedWidgetName];
        }
    }

    void CTextBox::Tooltip()
    {
        if (m_strTooltip.length() > 1 && !m_bIsGettingKey)
        {
            FGUI::DIMENSION dmTooltipTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strTooltip);

            FGUI::AREA arTooltipRegion = { (FGUI::INPUT.GetCursorPos().m_iX + 10), (FGUI::INPUT.GetCursorPos().m_iY + 10), (dmTooltipTextSize.m_iWidth + 10), (dmTooltipTextSize.m_iHeight + 10) };

            FGUI::RENDER.Outline(arTooltipRegion.m_iLeft, arTooltipRegion.m_iTop, arTooltipRegion.m_iRight, arTooltipRegion.m_iBottom, { 180, 95, 95 });
            FGUI::RENDER.Rectangle((arTooltipRegion.m_iLeft + 1), (arTooltipRegion.m_iTop + 1), (arTooltipRegion.m_iRight - 2), (arTooltipRegion.m_iBottom - 2), { 225, 90, 75 });
            FGUI::RENDER.Text(arTooltipRegion.m_iLeft + (arTooltipRegion.m_iRight / 2) - (dmTooltipTextSize.m_iWidth / 2),
                arTooltipRegion.m_iTop + (arTooltipRegion.m_iBottom / 2) - (dmTooltipTextSize.m_iHeight / 2), m_anyFont, { 245, 245, 245 }, m_strTooltip);
        }
    }

} // namespace FGUI
