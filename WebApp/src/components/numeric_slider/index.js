import { h } from 'preact';
import style from './style.css';
import {useSignal} from "@preact/signals";

const NumericSlider = (props) => {
    const current = useSignal(0);

    const valueChanged = (event) => {
        current.value = event.target.value;
    }

    return (
        <div>
            <input
                className={style.slider}
                type="range"
                id="slider"
                name="slider"
                min={props.minValue}
                max={props.maxValue}
                value={current.value}
                onChange={valueChanged}
            />
            <input
                className={style.spin_button}
                type="number"
                id="spinner"
                name="spinner"
                min={props.minValue}
                max={props.maxValue}
                value={current.value}
                onChange={valueChanged}
            />
        </div>
    );
}

export default NumericSlider;
