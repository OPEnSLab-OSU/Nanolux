import style from './style.css';
import React, { useState } from "react";
import { useSignal } from '@preact/signals';

/**
 * @brief Displays a selectable drop down list of LED strip patterns
 * 
 * @param initialID The pattern ID to display initially.
 * @param structure_ref The string reference to store values at.
 * @param update A function to update an external data structure.
 * @param patterns A list of patterns and their IDs.
 * 
 * @returns The UI element itself.
 */
const Patterns = ({
    initialID,
    structure_ref,
    update,
    patterns
}) => {

    // An object to store the current selected ID of the drop down.
    const current = useSignal(initialID);

    /**
     * @brief Creates a list of HTML list options based on the "patterns" parameter.
     */
    const patternOptions = patterns.map(p => {
        return <option key={p.index} value={p.index}>
            {p.name}
        </option>
    });

    /**
     * @brief Updates the selected ID and sends it to an external data structure.
     * @param event The event that stored the newly selected ID.
     */
    const handleSelection = async (event) => {
        current.value = Number(event.target.value);
        update(structure_ref, current.value);
    }

    const increment = () => {
        
        if (current.value == patternOptions.length - 1){
            current.value = 0;
        }
        else{
            current.value++;
        }
        //current.value = ((current.value + 1) % length(patternOptions));
        update(structure_ref, current.value);
    }

    const decrement = () => {
        
        if (current.value == 0){
            current.value = patternOptions.length - 1;
        }
        else{
            current.value--;
        }
        //current.value = ((current.value - 1) % length(patternOptions));
        update(structure_ref, current.value);
    }

    return (
        <div>
            <div>
                <label className={style.label} htmlFor="pattern-options">Current Pattern</label>
                <select className={style.select}
                        id="pattern-options"
                        value={initialID}
                        onChange={handleSelection}
                >
                    {patternOptions}
                </select>
                <button className={style.cycleBtn} onClick={decrement}>Prev</button>
                <button className={style.cycleBtn} onClick={increment}>Next</button>
            </div>
        </div>
    );
}

export default Patterns;
