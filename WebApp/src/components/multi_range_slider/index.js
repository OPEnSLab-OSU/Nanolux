import MultiRangeSlider from "multi-range-slider-react";

/**
 * @brief A NanoLux wrapper for the Multi Range Slider object.A
 * 
 * @param min The minimum value selectable by the slider.
 * @param max The maximum value selectable by the slider.
 * @param selectedLow The currently selected low value.
 * @param selectedHigh The currently selected high value
 * @param minRef The structure refrence for the minimum value.
 * @param maxRef The structure reference for the maximum value.
 * @param update A function which updates an external data structure.
 * 
 * @returns The UI element itself.
 */
const MultiRangeSliderWrapper = ({
    min,
    max,
    selectedLow,
    selectedHigh,
    minRef,
    maxRef,
    update
}) => {

    // SOURCE: https://github.com/developergovindgupta/multi-range-slider-react

    const handleInput = (e) => {
        update(minRef, e.minValue);
        update(maxRef, e.maxValue);
    };

    return (<div>

        <MultiRangeSlider
            min={min}
            max={max}
            step={50}
            minValue={selectedLow}
            maxValue={selectedHigh}
            onChange={(e) => {
                handleInput(e);
            }}
        />

    </div>)

}

export default MultiRangeSliderWrapper;