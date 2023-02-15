import { h } from 'preact';
import style from './style.css';
import {useSignal} from "@preact/signals";

const LedCount = ({
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
                <span className={style.label}>{label}</span>
            </div>
            <div>
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

export default LedCount;
