import { h } from 'preact';
import style from './style.css';
import {useSignal} from "@preact/signals";
import {useEffect} from "preact/hooks";

const NumericSlider = ({
    label,
    savedValue,
    min,
    max,
    onValueChanged
}) => {
    const current = useSignal(savedValue);

    useEffect(() => {
        if (savedValue) {
            current.value = savedValue;
        }
    }, [savedValue])

    const valueChanged = event => {
        current.value = event.target.value;
        onValueChanged(label, current.value);
    }

    return (
        <div>

            <div style="display: flex; align-items: center; justify-content: center;">
                <span className={style.info}>{label}: </span>
                <input
                    className={style.slider}
                    type="range"
                    id="slider"
                    name="slider"
                    min={min}
                    max={max}
                    value={current}
                    onChange={valueChanged}
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
