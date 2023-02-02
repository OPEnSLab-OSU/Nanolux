import style from './style.css';
import {useSignal} from "@preact/signals";

const NumericSlider = (props) => {
    const current = useSignal(0);

    const valueChanged = event => {
        current.value = event.target.value;
    }

    return (
        <div>
            <div>
                <span>{props.label}</span>
            </div>
            <div>
                <input
                    className={style.slider}
                    type="range"
                    id="slider"
                    name="slider"
                    min={props.minValue}
                    max={props.maxValue}
                    value={current}
                    onInput={valueChanged}
                />
                <input
                    className={style.spin_button}
                    type="number"
                    id="spinner"
                    name="spinner"
                    min={props.minValue}
                    max={props.maxValue}
                    value={current}
                    onChange={valueChanged}
                />
            </div>
        </div>
    );
}

export default NumericSlider;
