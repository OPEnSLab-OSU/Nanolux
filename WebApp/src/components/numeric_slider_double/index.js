// import style from './style.css';

const NumericSliderDouble = () => {
    return (
        <div className={style.range_container}>
            <div className={style.sliders_control}>
                <input id="fromSlider" type="range" value="10" min="0" max="100"/>
                <input id="toSlider" type="range" value="40" min="0" max="100"/>
            </div>
            <div class={style.form_control}>
                <div className={style.form_control_container}>
                    <div className={style.form_control_container__time}>Min</div>
                    <input className={style.form_control_container__time__input} type="number" id="fromInput" value="10" min="0" max="100"/>
                </div>
                <div className={style.form_control_container}>
                    <div className={style.form_control_container__time}>Max</div>
                    <input className={style.form_control_container__time__input} type="number" id="toInput" value="40" min="0" max="100"/>
                </div>
            </div>
        </div>
    );
}

export default NumericSliderDouble;
