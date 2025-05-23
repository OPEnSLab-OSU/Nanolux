import MultiRangeSlider from "audiolux-multi-range-slider";
import TooltipWrapper from "../tooltip/tooltip_wrapper";
import { useState } from "preact/hooks";

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
    tooltip,
    min,
    max,
    selectedLow,
    selectedHigh,
    minRef,
    maxRef,
    update
}) => {
    
    const getHueColor = (value) => `hsl(${(value / 255) * 360}, 100%, 50%)`;

    const [thumbMinColor, setThumbMinColor] = useState(getHueColor(selectedLow));
    const [thumbMaxColor, setThumbMaxColor] = useState(getHueColor(selectedHigh));

    const handleThumbs = (e) => {
        setThumbMinColor(getHueColor(e.minValue));
        setThumbMaxColor(getHueColor(e.maxValue));
    }

    const handleInput = (e) => {
        update(minRef, e.minValue);
        update(maxRef, e.maxValue);
    };

    return (
        <div>
            {tooltip && (
                <TooltipWrapper
                  id={tooltip.id}
                  content={tooltip.content}
                  offset={tooltip.offset}
                  style={tooltip.style}
                  label={'Color Range'}
                />
            )}
            <br/>
            <MultiRangeSlider
                min={min}
                max={max}
                step={50}
                ruler={false}
                minValue={selectedLow}
                maxValue={selectedHigh}
                onChange={(e) => {
                    handleInput(e);
                }}
                onInput={(e) => {
                    handleThumbs(e);
                }}
                barLeftColor="transparent"
                barInnerColor="transparent"
                barRightColor="transparent"
                thumbLeftColor={thumbMinColor}
                thumbRightColor={thumbMaxColor}
                style={{
                    border: "none",
                    boxShadow: "none",
                    background: '#FFFFFF',
                    color: '#000000'
                }}
            />
        </div>
    )
}

export default MultiRangeSliderWrapper;