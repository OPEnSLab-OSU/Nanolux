import style from './style.css';
import {useState} from "preact/hooks";

const SimpleSlider = (props) => {
    const [value, setValue] = useState(0);

    const valueChanged = (event) => {
        setValue(event.target.value);
    };

    return (
        <div>
            <input
                className={style.slider}
                type="range"
                id="slider"
                name="slider"
                min={props.minValue}
                max={props.maxValue}
                value={value}
                onChange={valueChanged}
            />
        </div>
    );
}

export default SimpleSlider;
