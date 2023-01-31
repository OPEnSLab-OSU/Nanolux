import { h } from 'preact';
import style from './style.css';
import {useSignal} from "@preact/signals";

const NumericSlider = (props) => {
    const current = useSignal(0);

    const valueChaged = (newValue) => {
        current.value = newValue;
    }

    return (
        <div>
            <input
                className={style.label}
                type="range"
                id="slider"
                name="slider"
                min={props.minValue}
                max={props.maxValue}
                value={current.value}
                onChange={valueChaged}
            />
            <input
                className={style.label}
                type="number"
                id="spinner"
                name="spinner"
                min={props.minValue}
                max={props.maxValue}
                value={current.value}
                onChange={valueChaged}
            />
        </div>
    );
}

export default NumericSlider;
