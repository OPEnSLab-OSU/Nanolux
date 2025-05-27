import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";

const TextInput = ({
                       inputPrompt,
                       commmitPrompt,
                       textValue,
                       onTextChange,
                       onTextCommit
                   }) => {
    const [text, setText] = useState("")

    useEffect(() => {
        setText(textValue)
    }, [textValue]);

    const handleInputChange = async (event) => {
        const newText = event.target.value;
        setText(newText);
        onTextChange(newText);
    }

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
