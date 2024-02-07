import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getPattern, getPatternList, savePattern} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import {LabelSpinner} from "../spinner";
import React, { useLayoutEffect } from "react";
import { useSignal } from '@preact/signals';

const Patterns = (
    patternList,
    initialID,
    structure_ref,
    update
) => {

    const current = useSignal(initialID);
    const [patterns, setPatterns] = useState({});

    useLayoutEffect(() => {
        // If patternList is null, use a list with only a blank pattern.
        // This is useful for avoiding crashes when running in dev mode
        // on a computer.
        if(patternList == {}){
            setPatterns({0 : {index: 0, name: "None"}});
        }else{
            setPatterns(patternList);
        }
    }, []);

    const handleSelection = async (event) => {
        current.value = event.target.value;
        update(structure_ref, current.value);
    }

    return (
        <div>
            <div>
                <label className={style.label} htmlFor="pattern-options">Current Primary Pattern</label>
                <select className={style.label}
                        id="pattern-options"
                        value={current}
                        onChange={handleSelection}
                >
                </select>
            </div>
        </div>
    );
}

export default Patterns;
