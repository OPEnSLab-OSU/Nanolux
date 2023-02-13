import { h } from 'preact';
import style from './style.css';
import {useSignal} from "@preact/signals";

const NumericSlider = ({
    label,
    savedValue,
    min,
    max
}) => {
    const current = useSignal(savedValue);

    const valueChanged = event => {
        current.value = event.target.value;
    }

    return (
        <div>
            <div>
                <span>{label}</span>
            </div>
            <div>
                <input
                    className={style.slider}
                    type="range"
                    id="slider"
                    name="slider"
                    min={min}
                    max={max}
                    value={current}
                    onInput={valueChanged}
                />
                <input
                    className={style.spin_button}
                    type="number"
                    id="spinner"
                    name="spinner"
                    min={min}
                    max={max}
                    value={current}
                    onChange={valueChanged}
                />
            </div>
        </div>
    );
}

export default NumericSlider;
