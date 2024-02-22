import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getPatternList} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import {LabelSpinner} from "../spinner";
import React, { useLayoutEffect } from "react";
import { useSignal } from '@preact/signals';

const Patterns = ({
    initialID,
    structure_ref,
    update,
    patterns
}) => {

    const current = useSignal(initialID);

    const patternOptions = patterns.map(p => {
        return <option key={p.index} value={p.index}>
            {p.name}
        </option>
    });

    const handleSelection = async (event) => {
        current.value = Number(event.target.value);
        update(structure_ref, current.value);
    }

    return (
        <div>
            <div>
                <label className={style.label} htmlFor="pattern-options">Current Pattern</label>
                <select className={style.label}
                        id="pattern-options"
                        value={initialID}
                        onChange={handleSelection}
                >
                    {patternOptions}
                </select>
            </div>
        </div>
    );
}

export default Patterns;
