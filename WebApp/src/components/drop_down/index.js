import style from './style.css';
import {useEffect} from "preact/hooks";
import {useSignal} from "@preact/signals";


const DropDown = ({
    label,
    api_key,
    optionsList,
    savedValue,
    onValueChanged
}) => {

    const current = useSignal(savedValue);
    

    useEffect(() => {
        if (savedValue) {
            current.value = savedValue;
        }
    }, [savedValue])

    
    
    const  valueChanged = async event => {
        current.value = event.target.value;
        onValueChanged(api_key, current.value);
    }

    return (
        <div>
            <div>
                <label className={style.label} htmlFor="options">{label}</label>
                <select className={style.label}
                        id="options"
                        value={savedValue}
                        onChange={valueChanged}
                >
                {optionsList}
                </select>
            </div>
        </div>
    );
}

export default DropDown;
