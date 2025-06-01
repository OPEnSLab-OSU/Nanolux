import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";

/**
 * @brief A UI element for single-line text input with a commit button.
 *
 * This component maintains local state for the text field, synchronizes it
 * with an external `textValue`, calls `onTextChange` on every keystroke, and
 * calls `onTextCommit` when the user clicks the commit button.
 *
 * @param inputPrompt    The label text displayed above the text input.
 * @param commmitPrompt  The label for the commit button.
 * @param textValue      The externally controlled text value to display.
 * @param onTextChange   Callback invoked on each change to the text (receives new text).
 * @param onTextCommit   Callback invoked when the commit button is clicked (receives current text).
 *
 * @returns The TextInput UI element.
 */
const TextInput = ({
                       inputPrompt,
                       commmitPrompt,
                       textValue,
                       onTextChange,
                       onTextCommit
                   }) => {
    // Local state to hold the current text in the input field.
    const [text, setText] = useState("")

    /**
     * @brief Synchronize local `text` state whenever `textValue` prop changes.
     */
    useEffect(() => {
        setText(textValue)
    }, [textValue]);

    /**
     * @brief Handles user typing in the input field.
     *        Updates local state and notifies the parent via `onTextChange`.
     *
     * @param event The input change event, with `event.target.value` as the new text.
     */
    const handleInputChange = async (event) => {
        const newText = event.target.value;
        setText(newText);
        onTextChange(newText);
    }

    /**
     * @brief Called when the commit button is clicked.
     *        Calls `onTextCommit` with the current local text.
     */
    const commitText = async () => {
        onTextCommit(text);
    }

    return (
        <div role='group' aria-labelledby='text-input-label'>
            <label id='text-input-label' className={style.label} htmlFor="text-input">{inputPrompt}</label>
            <input className={style.label}
                   id="text-input"
                   type="text"
                   value={text}
                   onChange={handleInputChange}
            />
            <div>
                <button
                  type='button'
                  className={style.formButton}
                  id="show-password"
                  onClick={commitText}
                >
                    {commmitPrompt}
                </button>
            </div>
        </div>
    );
}

export default TextInput;
